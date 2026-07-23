//
// Created by andrew on 07.03.26.
//

#pragma once
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <arpa/inet.h>
#include <vector>
#include "ServerInfo.h"
#include "Client.h"
class ServerSocket {
public:
    explicit ServerSocket(ServerInfo serverInfo) : serverInfo(std::move(serverInfo)) {
        this->init();
    };
    std::shared_ptr<Client> waitForConnection();
    const ServerInfo& getServerInfo() const { return serverInfo; }
private:
    void init();
    int fd = -1;
    int clientFd = -1;
    struct sockaddr_in servAddr{};
    struct sockaddr_in cliAddr{};
    ServerInfo serverInfo{};

};
