#pragma once
#include <iostream>
#include <netinet/in.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <spdlog/spdlog.h>
#include "ServerInfo.h"
#include "ClientIdentifier.h"
#include "ServerAuthResponseHeader.h"
#include "ClientAuthHeader.h"

class AbstractClient {
public:
    explicit AbstractClient(ServerInfo info) : info(std::move(info)) {this->init();}
    void connectToServer();
    void sendAuthRequest();
    bool authenticationSuccessful() const;
    void disconnectFromServer() const;
    void init();
    virtual void run() = 0;
protected:
    struct sockaddr_in servAddr{};
    ServerInfo info;
    int fd = -1;
    uint64_t id = 0x0;
};
