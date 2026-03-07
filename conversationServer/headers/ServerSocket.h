//
// Created by andrew on 07.03.26.
//

#pragma once
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include <vector>
#include "ServerInfo.h"
class ServerSocket {
public:
    struct Client {
        int clientFd = -1;
        std::string clientIP;
        int port = -1;
    };
    explicit ServerSocket(ServerInfo serverInfo) : serverInfo(std::move(serverInfo)) {
        this->init();
    };
    Client waitForConnection();
private:
    void init();
    int fd = -1;
    int clientFd = -1;
    struct sockaddr_in servAddr{};
    struct sockaddr_in cliAddr{};
    ServerInfo serverInfo{};

};