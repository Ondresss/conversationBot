#include "../headers/AbstractClient.h"


void AbstractClient::init() {
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) throw std::runtime_error("Error creating socket\n");
    spdlog::debug("TCP audio socket intiliazed\n");
    std::memset(&this->servAddr,0 ,sizeof(struct sockaddr_in));
    this->servAddr.sin_family = AF_INET;
    const int res = inet_pton(AF_INET, this->info.ip.c_str(), &this->servAddr.sin_addr);
    if (res == 0) throw std::runtime_error("Invalid IPv4 address\n");
    if (res == -1) throw std::runtime_error("Error while converting IPv4 string to AF_INET\n");
    this->servAddr.sin_port = htons(this->info.port);
    spdlog::info("Server address for connection is: {} with port {}",this->info.ip,this->info.port);
}

void AbstractClient::sendAuthRequest() {
    ClientAuthHeader authHeader{};
    authHeader.id = this->id;
    ssize_t totaBytes = sizeof(ClientAuthHeader);
    ssize_t bytesSent = 0;
    while (bytesSent < totaBytes) {
        ssize_t res = send(this->fd, &authHeader + bytesSent, totaBytes - bytesSent, 0);
        if (res == -1) throw std::runtime_error("Error sending auth request\n");
        bytesSent += res;
    }
    spdlog::info("Auth request sent to the server with id {}", this->id);
}

bool AbstractClient::authenticationSuccessful() const {
    ServerAuthResponseHeader header{};
    ssize_t totalBytes = sizeof(ServerAuthResponseHeader);
    ssize_t bytesReceived = 0;
    while (bytesReceived < totalBytes) {
        ssize_t res = recv(this->fd, &header + bytesReceived, totalBytes - bytesReceived, 0);
        if (res == -1) throw std::runtime_error("Error receiving auth response\n");
        bytesReceived += res;
    }
    return header.status == ServerAuthStatus::Success;
}

void AbstractClient::connectToServer() {
    if (connect(this->fd, reinterpret_cast<struct sockaddr*>(&this->servAddr), sizeof(this->servAddr)) != 0) {
        throw std::runtime_error("ConversationClient::connectToServer(): connection with the server failed...\n");
    }
    spdlog::info("Connected to the server with TCP audio socket");
    this->id = ClientIdentifier::getIdentifier();
    spdlog::info("About to send auth request with id {}", this->id);
    this->sendAuthRequest();
    spdlog::info("Auth request sent to the server with id {}", this->id);
    if(!this->authenticationSuccessful()) {
        spdlog::info("Authentication failed");
        throw std::runtime_error("Authentication failed");
    }
}
void AbstractClient::disconnectFromServer() const {
    close(this->fd);
    spdlog::warn("Disconnected from the server");
}
