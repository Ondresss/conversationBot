//
// Created by andrew on 07.03.26.
//

#include "../headers/ConversationServer.h"

#include "../headers/ClientHeader.h"

#include <unistd.h>
#include <rtaudio/RtAudio.h>

void ConversationServer::run() {
    try {
        if (!this->serverSocket) throw std::runtime_error("ConversationServer::run(): serverSocket is null");
        while (true) {
            spdlog::info("Waiting for new client....");
            auto client = this->serverSocket->waitForConnection();
            spdlog::info("New client joined with IP {}",client->clientIP);
            this->clientThreads.emplace_back(&ConversationServer::handleClient, this,client);
        }
    } catch (std::exception& e){
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void ConversationServer::handleClient(std::shared_ptr<ServerSocket::Client> client) {
    spdlog::info("Handling the client......");
    const SherpaOnnxOnlineStream* clientStream = nullptr;
    if (this->speechToTextConverter->isOnline()) clientStream = this->speechToTextConverter->createStream();

    std::vector<float> audioBuffer;
    try {
        while (true) {
            auto currentAudio = this->readAudioFromClient(client);
            if (currentAudio.empty()) {
                spdlog::info("Read full sentence");
                if (audioBuffer.size() <= 8000) {
                    spdlog::error("Client sentence was too short!");
                    this->writeResponse(client,{},ServerStatus::TOOSHORT);
                    audioBuffer.clear();
                    continue;
                }
                std::string currentText = this->speechToTextConverter->processAudioChunk(clientStream, audioBuffer);
                spdlog::info("Audio processed");
                if (!currentText.empty() && currentText.length() >= 20) {
                    spdlog::info("Input Text: {}", currentText);
                    std::string response = this->llmGateway->askLLM(currentText);
                    spdlog::info("LLM RESPONSE: {}", response);
                    spdlog::debug("Starting Piper synthesis...");
                    try {
                        this->textToSpeechConverter->convertTextToSpeech(response);
                    } catch (const std::exception& e) {
                        spdlog::error("ERR: Exception during TTS");
                        spdlog::error("{}",e.what());
                        close(client->clientFd);
                        spdlog::info("Closed connection with client");
                        return;
                    }
                    std::vector<int16_t> audioOut = this->textToSpeechConverter->getOutput();
                    spdlog::debug("Piper produced {} samples ({} bytes).", audioOut.size(), audioOut.size() * sizeof(int16_t));

                    this->writeResponse(client, audioOut,ServerStatus::OK);
                    spdlog::debug("Response written to socket.");
                } else {
                    spdlog::error("Empty text response!");
                    this->writeResponse(client,{},ServerStatus::EMPTY_RESPONSE);
                }
                audioBuffer.clear();
            } else {
                audioBuffer.insert(audioBuffer.end(), currentAudio.begin(), currentAudio.end());
            }
        }
    } catch (const std::exception& e) {
        spdlog::error("void ConversationServer::handleClient(std::shared_ptr<ServerSocket::Client> client): {}",e.what());
    }
    this->speechToTextConverter->destroyStream(clientStream);
    spdlog::debug("Stream destroyed");
}
std::vector<float> ConversationServer::readAudioFromClient(const std::shared_ptr<ServerSocket::Client>& client) {
    std::vector<float> fullAudio;
    ClientHeader clientHeader{};
    char* headerPtr = reinterpret_cast<char*>(&clientHeader);
    ssize_t headerBytesLeft = sizeof(clientHeader);
    while (headerBytesLeft > 0) {
        ssize_t n = read(client->clientFd, headerPtr, headerBytesLeft);
        if (n == 0) throw std::runtime_error("ConversationServer::readAudioFromClient(): Client disconnected while sending header");
        if (n == -1) throw std::runtime_error("ConversationServer::readAudioFromClient(): Error while reading header from client " + std::string(strerror(errno)) );
        headerBytesLeft -= n;
        headerPtr += n;
    }
    size_t numSamples = clientHeader.packetLen / sizeof(float);
    std::vector<float> packetData(numSamples);
    char* dataPtr = reinterpret_cast<char*>(packetData.data());
    ssize_t dataBytesLeft = clientHeader.packetLen;
    if (clientHeader.status == 3) {
        return {};
    }
    while (dataBytesLeft > 0) {
        ssize_t n = read(client->clientFd, dataPtr, dataBytesLeft);
        if (n <= 0) return fullAudio;
        dataBytesLeft -= n;
        dataPtr += n;
    }
    fullAudio.insert(fullAudio.end(), packetData.begin(), packetData.end());
    return fullAudio;
}

void ConversationServer::writeResponse(const std::shared_ptr<ServerSocket::Client>& client,const std::vector<std::int16_t>& soundBytes,ServerStatus status) {
    spdlog::debug("Writing response back to client");
    ServerHeader header{};
    header.status = static_cast<uint32_t>(status);
    header.totalLen = soundBytes.size() * sizeof(std::int16_t);
    char* headerPtr = reinterpret_cast<char*>(&header);
    ssize_t headerBytesLeft = sizeof(header);
    while (headerBytesLeft > 0) {
        ssize_t n = write(client->clientFd, headerPtr, headerBytesLeft);
        if (n == 0) throw std::runtime_error("ConversationServer::writeResponse(): Client disconnected while sending header");
        if (n == -1) throw std::runtime_error("ConversationServer::writeResponse(): Error while reading header from client " + std::string(strerror(errno)) );
        headerBytesLeft -= n;
        headerPtr += n;
    }

    auto dataPtr = reinterpret_cast<const char*>(soundBytes.data());
    ssize_t dataBytesLeft = header.totalLen;
    while (dataBytesLeft > 0) {
        ssize_t n = write(client->clientFd, dataPtr, dataBytesLeft);
        if (n <= 0) std::cout << "Client disconnected or ended\n";
        dataBytesLeft -= n;
        dataPtr += n;
    }
    spdlog::debug("Wrote response back to client");
}
