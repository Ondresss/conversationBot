//
// Created by andrew on 05.03.26.
//

#pragma once
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>
#include <iostream>
#include "AudioPacket.h"
#include "ClientIdentifier.h"
#include "ServerAuthResponseHeader.h"
#include "ServerConversationHeader.h"
#include "ClientAuthHeader.h"
#include "ClientConversationHeader.h"

class ConversationClient {
public:
    struct ServerInfo {
        int port = -1;
        std::string ip;
    };

    explicit ConversationClient(ServerInfo serverInfo) : serverInfo(std::move(serverInfo)) {
        this->init();
        this->id = ClientIdentifier::getIdentifier();
    };
    void connectToServer();
    void sendAuthRequest();
    bool authenticationSuccessful() const;
    void disconnectFromServer() const;
    void sendAudioPacket(const AudioPacket& audioPacket);
    std::tuple<const std::vector<std::int16_t>&,uint32_t> getResponseFromServer();

private:
    ServerInfo serverInfo;
    int fd = -1;
    std::vector<std::int16_t> responseBuffer;
    struct sockaddr_in servAddr{};
    uint64_t id = 0x0;
    void init();
};
