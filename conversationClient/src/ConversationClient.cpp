//
// Created by andrew on 05.03.26.
//
#include  "../headers/ConversationClient.h"

#include <unistd.h>

#include "../headers/AudioPacket.h"

void ConversationClient::init() {
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) throw std::runtime_error("Error creating socket\n");
    std::memset(&this->servAddr,0 ,sizeof(struct sockaddr_in));
    this->servAddr.sin_family = AF_INET;
    const int res = inet_pton(AF_INET, this->serverInfo.ip.c_str(), &this->servAddr.sin_addr);
    if (res == 0) throw std::runtime_error("Invalid IPv4 address\n");
    if (res == -1) throw std::runtime_error("Error while converting IPv4 string to AF_INET\n");
    this->servAddr.sin_port = htons(this->serverInfo.port);
}


void ConversationClient::connectToServer() {
    if (connect(this->fd, reinterpret_cast<struct sockaddr*>(&this->servAddr), sizeof(this->servAddr)) != 0) {
        throw std::runtime_error("ConversationClient::connectToServer(): connection with the server failed...\n");
    }
    std::cout << "ConversationClient::connectToServer(): Connected to the server" << std::endl;

}

void ConversationClient::disconnectFromServer() const {
    close(this->fd);
}

void ConversationClient::sendAudioPacket(const AudioPacket& audioPacket) {
    ClientHeader clientHeader{};
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
    if (!audioPacket.samples.empty()) {
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
    }
}

 std::tuple<const std::vector<std::int16_t>&,uint32_t> ConversationClient::getResponseFromServer() {
    std::cout << "Reading response from the server....\n";
    ServerHeader serverHeader{};
    auto serverHeaderPtr = reinterpret_cast<char*>(&serverHeader);
    ssize_t headerBytesLeft = sizeof(ServerHeader);
    while (headerBytesLeft > 0) {
        const ssize_t headerBytesRead = read(this->fd, serverHeaderPtr, headerBytesLeft);
        if (headerBytesRead == -1) throw std::runtime_error("ConversationClient::getResponseFromServer(): Error reading from the socket\n");
        headerBytesLeft -= headerBytesRead;
        serverHeaderPtr += headerBytesRead;
    }
    size_t numSamples = serverHeader.totalLen / sizeof(std::int16_t);

    this->responseBuffer.resize(numSamples);

    char* dataPtr = reinterpret_cast<char*>(this->responseBuffer.data());
    ssize_t bytesLeft = serverHeader.totalLen;
    while (bytesLeft > 0) {
        const ssize_t bytesRead = read(this->fd, dataPtr, bytesLeft);
        if (bytesRead == 0) break;
        bytesLeft -= bytesRead;
        dataPtr += bytesRead;
    }
    std::cout << "INFO: read response from the server\n";
    return {this->responseBuffer,serverHeader.status};
}
