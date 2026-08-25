//
// Created by andrew on 05.03.26.
//
#include  "../headers/ConversationClient.h"
#include <cstdint>
#include <mutex>
#include <spdlog/spdlog.h>

void ConversationClient::run() {
    if (!this->audioHandler) throw std::runtime_error("ConversationBot::run(): audioHandler is null");
    this->connectToServer();
    this->audioHandler->startRecording();
    this->audioHandler->startPlayback();
    int noSilencePackets = 0;
    while (noSilencePackets < 100000) {
        if (!this->audioHandler->hasPackets()) continue;
        auto audioPacket = this->audioHandler->getNextAudioPacket();
        switch (audioPacket.type) {
        case AudioType::SILENCE:
            noSilencePackets++;
            if (!audioPacket.samples.empty()) this->sendAudioPacket(audioPacket);
            break;
        case AudioType::STARTOFSPEECH:
            noSilencePackets = 0;
            spdlog::debug("Start of speech detected\n");
            this->sendAudioPacket(audioPacket);
            break;
            case AudioType::ENDOFSPEECH: {
                noSilencePackets = 0;
                spdlog::debug("End of speech detected");
                this->sendAudioPacket(audioPacket);

                bool streaming = true;
                bool disconnected = false;
                while (streaming) {
                    auto [response, status] = this->getResponseFromServer();
                    if(status == ServerStatus::DISCONNECT) {
                        streaming = false;
                        disconnected = true;
                        continue;
                    }

                    if (status == ServerStatus::TOO_SHORT) {
                        spdlog::warn("Sentence was too short");
                        streaming = false;
                        continue;
                    }
                    if (status == ServerStatus::EMPTY_RESPONSE) {
                        spdlog::info("Received empty response");
                        streaming = false;
                        continue;
                    }

                    if (!response.empty()) {
                        this->audioHandler->setPlaybackContextData(response);
                    }

                    if (status == ServerStatus::OK) {
                        streaming = false;
                    }
                }
                if(disconnected) {
                    spdlog::warn("Disconnected from server");
                    break;
                }

                auto& ctx = this->audioHandler->getPlaybackContextData();
                {
                    std::lock_guard lk(ctx.mtx);
                    ctx.doneTalking = false;
                    ctx.canTalk = true;
                    spdlog::debug("Master notified worker to start talking");
                }
                ctx.cv.notify_one();

                {
                    std::unique_lock lk(ctx.mtx);
                    ctx.cv.wait(lk, [&ctx] { return ctx.doneTalking; });
                }
                spdlog::info("Done talking");
                ctx.doneTalking = false;
                ctx.canTalk = false;
                break;
            }
        case AudioType::SPEECH:
            noSilencePackets = 0;
            spdlog::debug("Speech detected");
            this->sendAudioPacket(audioPacket);
            break;
        default:
            spdlog::warn("Unrecognized speech pattern");
        }

    }
    this->audioHandler->stopRecording();
    this->disconnectFromServer();
}

void ConversationClient::sendAudioPacket(const AudioPacket& audioPacket) {
    ClientConversationHeader clientHeader{};
    clientHeader.status = static_cast<uint32_t>(audioPacket.type);
    clientHeader.packetLen = audioPacket.samples.size() * sizeof(float);
    const char* headerPtr = reinterpret_cast<const char*>(&clientHeader);
    ssize_t headerLeft = sizeof(clientHeader);

    while (headerLeft > 0) {
        ssize_t sent = write(this->fd, headerPtr, headerLeft);
        if (sent == -1) throw std::runtime_error("ConversationClient::sendAudioPacket(const AudioPacket& audioPacket): Error writing to the socket\n");
        headerLeft -= sent;
        headerPtr += sent;
    }
    spdlog::debug("Sent tcp audio header -> [status,length] = [{},{}]",clientHeader.status,clientHeader.packetLen);
    if (!audioPacket.samples.empty()) {
        spdlog::debug("TCP audio packet wasnt empty");
        const char* dataPtr = reinterpret_cast<const char*>(audioPacket.samples.data());
        ssize_t dataLeft = audioPacket.samples.size() * sizeof(float);

        while (dataLeft > 0) {
            ssize_t sent = write(this->fd, dataPtr, dataLeft);
            if (sent <= 0) {
                if (errno == EINTR) continue;
                throw std::runtime_error("Socket error during audio data send");
            }
            dataLeft -= sent;
            dataPtr += sent;
        }
        spdlog::debug("Sent nonempty TCP packet");
    }
}

 std::tuple<const std::vector<std::int16_t>&,ServerStatus> ConversationClient::getResponseFromServer() {
    spdlog::debug("Reading response from the server");
    ServerConversationHeader serverHeader{};
    auto serverHeaderPtr = reinterpret_cast<char*>(&serverHeader);
    ssize_t headerBytesLeft = sizeof(ServerConversationHeader);
    while (headerBytesLeft > 0) {
        const ssize_t headerBytesRead = read(this->fd, serverHeaderPtr, headerBytesLeft);
        if (headerBytesRead == -1) throw std::runtime_error("ConversationClient::getResponseFromServer(): Error reading from the socket\n");
        headerBytesLeft -= headerBytesRead;
        serverHeaderPtr += headerBytesRead;
    }
    size_t numSamples = serverHeader.totalLen / sizeof(std::int16_t);

    this->responseBuffer.resize(numSamples);

    spdlog::debug("Server header read [totalLength,status]=[{},{}]",serverHeader.totalLen,static_cast<uint32_t>(serverHeader.status));
    char* dataPtr = reinterpret_cast<char*>(this->responseBuffer.data());
    ssize_t bytesLeft = serverHeader.totalLen;
    while (bytesLeft > 0) {
        const ssize_t bytesRead = read(this->fd, dataPtr, bytesLeft);
        if (bytesRead == 0) break;
        bytesLeft -= bytesRead;
        dataPtr += bytesRead;
    }
    spdlog::debug("Read response from the server");
    return {this->responseBuffer,serverHeader.status};
}
