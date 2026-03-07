//
// Created by andrew on 07.03.26.
//

#pragma once
#include <memory>
#include "ServerInfo.h"
#include <vector>
#include "ServerSocket.h"
#include <thread>
#include <iostream>
class ConversationServer {
public:
    explicit ConversationServer(ServerInfo serverInfo) {
        this->serverSocket = std::make_shared<ServerSocket>(serverInfo);
    };
    ~ConversationServer() {
        for (auto& th : this->clientThreads) {
            if (th.joinable()) {
                th.join();
            }
        }
    }
    void run();
    void handleClient(std::shared_ptr<ServerSocket::Client> client);
    std::vector<float> readAudioFromClient(const std::shared_ptr<ServerSocket::Client>& client);
    void writeResponse(const std::string& text);
private:
    std::shared_ptr<ServerSocket> serverSocket;
    std::vector<std::thread> clientThreads;
};
