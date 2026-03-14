//
// Created by andrew on 07.03.26.
//

#include "../headers/ConversationServer.h"

#include "../headers/ClientHeader.h"

#include <unistd.h>
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
}void ConversationServer::handleClient(std::shared_ptr<ServerSocket::Client> client) {
    const SherpaOnnxOnlineStream* clientStream = this->speechToTextConverter->createStream();

    try {
        while (true) {
            auto audioBuffer = this->readAudioFromClient(client);
            if (!audioBuffer.empty()) {
                std::string currentText = this->speechToTextConverter->processAudioChunk(clientStream, audioBuffer);
                if (!currentText.empty()) {
                    std::cout << "\rText: " << currentText << std::flush;
                }
            } else {
                this->speechToTextConverter->destroyStream(clientStream);
                clientStream = this->speechToTextConverter->createStream();
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

void ConversationServer::writeResponse(const std::string& text) {
}
