//
// Created by andrew on 07.03.26.
//
#include "../headers/ServerSocket.h"

#include <iostream>

void ServerSocket::init() {
    this->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->fd == -1) throw std::runtime_error("ServerSocket::init(): Error creating socket");
    int opt = 1;
    setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    this->servAddr.sin_family = AF_INET;
    inet_pton(AF_INET,this->serverInfo.ip.c_str(),&this->servAddr.sin_addr);
    this->servAddr.sin_port = htons(this->serverInfo.port);

    if (bind(this->fd, reinterpret_cast<struct sockaddr*>(&this->servAddr), sizeof(this->servAddr)) < 0) {
        throw std::runtime_error("ServerSocket::init(): Bind failed - port is probably occupied");
    }
    if (listen(this->fd, 10) < 0) {
        throw std::runtime_error("ServerSocket::init(): Listen failed");
    }
    std::cout << "Server socket started on IP and PORT: " << this->serverInfo.ip << ", " << this->serverInfo.port << std::endl;
}

std::shared_ptr<ServerSocket::Client> ServerSocket::waitForConnection() {
    socklen_t addrLen = sizeof(this->cliAddr);
    int currentClientFd = accept(this->fd, reinterpret_cast<struct sockaddr*>(&this->cliAddr), &addrLen);
    if (currentClientFd == -1) {
        throw std::runtime_error("ServerSocket::waitForConnection(): accept failed");
    }
    auto clientSocket = std::make_shared<Client>();
    char ipStr[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(this->cliAddr.sin_addr), ipStr, INET_ADDRSTRLEN)) {
        clientSocket->clientIP = std::string(ipStr);
    }
    clientSocket->clientFd = currentClientFd;
    clientSocket->port = ntohs(this->cliAddr.sin_port);
    return clientSocket;
}