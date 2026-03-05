//
// Created by andrew on 05.03.26.
//

#pragma once
#include <string>
class ConversationClient {
public:
    struct ServerInfo {
        int port = -1;
        std::string ip;
    };
    explicit ConversationClient(ServerInfo serverInfo) : serverInfo(std::move(serverInfo))  {};

    void connectToServer();
    void disconnectFromServer();
    void sendBinary(const float* data,std::size_t len);
    void getTextFromServer();

private:
    ServerInfo serverInfo;
    int fd = -1;
    std::string recievedMessage;
};