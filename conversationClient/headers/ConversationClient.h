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

class ConversationClient {
public:
    struct ServerInfo {
        int port = -1;
        std::string ip;
    };
    #pragma pack(push, 1)
    struct ClientHeader {
        uint32_t status = 0x0;
        uint32_t packetLen = 512;
    };
    struct ServerHeader {
        uint32_t status = 0x0;
        uint32_t totalLen = 512;
    };
    #pragma pack(pop)
    explicit ConversationClient(ServerInfo serverInfo) : serverInfo(std::move(serverInfo)) {
        this->init();
    };
    void connectToServer();
    void disconnectFromServer() const;
    void sendAudioPacket(const AudioPacket& audioPacket);
    std::string getTextFromServer();

private:
    ServerInfo serverInfo;
    int fd = -1;
    std::string recievedMessage;
    struct sockaddr_in servAddr{};

    void init();
};