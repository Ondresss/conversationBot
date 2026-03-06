//
// Created by andrew on 05.03.26.
//
#include  "../headers/ConversationClient.h"

#include <unistd.h>

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
        throw std::runtime_error("connection with the server failed...\n");
    }
    std::cout << "Connected to the server" << std::endl;

}

void ConversationClient::disconnectFromServer() const {
    close(this->fd);
}

void ConversationClient::sendBinary(const float* data, std::size_t noBytes) const {
    const auto bytes = reinterpret_cast<const char*>(data);
    std::size_t bytesLeft = noBytes;
    while (bytesLeft > 0) {
        const std::size_t bytesSent = write(this->fd, bytes, bytesLeft);
        if (bytesSent == -1) throw std::runtime_error("Error writing to the socket\n");
        bytesLeft -= bytesSent;
    }
}

std::string ConversationClient::getTextFromServer() {
    ServerHeader serverHeader{};
    auto serverHeaderPtr = reinterpret_cast<char*>(&serverHeader);
    ssize_t headerBytesLeft = sizeof(ServerHeader);
    while (headerBytesLeft > 0) {
        const ssize_t headerBytesRead = read(this->fd, serverHeaderPtr, headerBytesLeft);
        if (headerBytesRead == -1) throw std::runtime_error("Error reading from the socket\n");
        headerBytesLeft -= headerBytesRead;
        serverHeaderPtr += headerBytesRead;
    }
    ssize_t bytesLeft = serverHeader.totalLen;
    auto BUFFER = new char[bytesLeft+1];
    std::memset(BUFFER, 0, bytesLeft+1);
    auto currentPtr = BUFFER;
    while (bytesLeft > 0) {
        const ssize_t bytesRead = read(this->fd, currentPtr, bytesLeft);
        if (bytesRead == -1) throw std::runtime_error("Error reading from the socket\n");
        if (bytesRead == 0) {
            std::cout << "Server sent 0 - Disconnected\n";
            break;
        }
        bytesLeft -= bytesRead;
        currentPtr += bytesRead;
    }
    this->recievedMessage = std::string(BUFFER);
    delete[] BUFFER;
    return this->recievedMessage;
}
