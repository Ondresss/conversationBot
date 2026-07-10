//
// Created by andrew on 4/18/26.
//

#include  "../headers/Client.h"
#include "../headers/ServerType.h"
#include <mutex>
#include <shared_mutex>

void Client::disconnect(ServerType type) {
    std::unique_lock<std::shared_mutex> lock(this->mutex);
    this->isConnected = false;
    switch (type) {
        case ServerType::Conversation:
            if(this->descriptors.audioFd == -1) throw std::runtime_error("Client already disconnected from audio server");
            this->descriptors.audioFd = -1;
            break;
        case ServerType::Image:
            if(this->descriptors.videoFd == -1) throw std::runtime_error("Client already disconnected from image server");
            close(this->descriptors.videoFd);
            this->descriptors.videoFd = -1;
            break;
        default:
            throw std::runtime_error("Invalid server type");
    }
}

nlohmann::json Client::serialize() {
    nlohmann::json j;
    j["id"] = std::to_string(this->id);
    j["ip"] = this->clientIP;
    j["connected"] = this->isConnected;
    nlohmann::json conversation = nlohmann::json::array();

    for (const auto& entry : this->historyList) {
        conversation.push_back({
            {"question", entry.question},
            {"answer", entry.answer}
        });
    }

    j["history"] = conversation;

    return j;
}
