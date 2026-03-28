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
            std::cout << "Waiting for new client...." << std::endl;
            auto client = this->serverSocket->waitForConnection();
            std::cout << "New client joined" << std::endl;
            this->clientThreads.emplace_back(&ConversationServer::handleClient, this,client);
        }
    } catch (std::exception& e){
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void ConversationServer::handleClient(std::shared_ptr<ServerSocket::Client> client) {
    const SherpaOnnxOnlineStream* clientStream = nullptr;
    if (this->speechToTextConverter->isOnline()) clientStream = this->speechToTextConverter->createStream();

    std::vector<float> audioBuffer;
    try {
        while (true) {
            auto currentAudio = this->readAudioFromClient(client);
            if (currentAudio.empty()) {
                std::string currentText = this->speechToTextConverter->processAudioChunk(clientStream, audioBuffer);
                if (!currentText.empty() && currentText.length() >= 20) {
                    std::cout << "\rText: " << currentText << std::flush << "\n";
                    std::string response = this->llmGateway->askLLM(currentText);
                    std::cout << "LLM RESPONSE: " << response << std::endl;
                    std::cout << "DEBUG: Starting Piper synthesis..." << std::endl;
                    try {
                        this->textToSpeechConverter->convertTextToSpeech(response);
                    } catch (const std::exception& e) {
                        std::cerr << "ERR: Exception during TTS\n";
                        std::cerr << e.what() << std::endl;
                        close(client->clientFd);
                        return;
                    }
                    std::vector<int16_t> audioOut = this->textToSpeechConverter->getOutput();
                    std::cout << "DEBUG: Piper produced " << audioOut.size() << " samples ("
                              << audioOut.size() * 2 << " bytes)." << std::endl;

                    this->writeResponse(client, audioOut);
                    std::cout << "DEBUG: Response written to socket." << std::endl;
                }
                audioBuffer.clear();
            } else {
                audioBuffer.insert(audioBuffer.end(), currentAudio.begin(), currentAudio.end());
            }
        }
    } catch (...) { /* cleanup */ }
    this->speechToTextConverter->destroyStream(clientStream);
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

void ConversationServer::writeResponse(const std::shared_ptr<ServerSocket::Client>& client,const std::vector<std::int16_t>& soundBytes) {
    if (soundBytes.empty()) throw std::runtime_error("ConversationServer::writeResponse(): SoundBytes is empty");
    ServerHeader header{};
    header.status = 1;
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
    std::cout << "Wrote response to the client" << std::endl;
}
