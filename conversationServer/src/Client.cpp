//
// Created by andrew on 4/18/26.
//

#include  "../headers/Client.h"
#include "../headers/ServerType.h"
#include <mutex>
#include <shared_mutex>
#include <spdlog/spdlog.h>

void Client::initializeSession(int secs) {
    if (session) {
        spdlog::warn("Trying to initalize already initialized session");
        session->refreshSession();
    }
    else session = std::make_unique<ConversationSession>(secs);
}

void Client::disconnect(ServerType type) {
    std::unique_lock<std::shared_mutex> lock(this->mutex);
    spdlog::debug("Client::disconnect -> Lock acquired");
    this->isConnected = false;
    switch (type) {
        case ServerType::Conversation:
            if(this->descriptors.audioFd == -1) throw std::runtime_error("Client already disconnected from audio server");
            this->descriptors.audioFd = -1;
            spdlog::warn("Client with id {} disconnected from audio server", this->id);
            break;
        case ServerType::Image:
            if(this->descriptors.videoFd == -1) throw std::runtime_error("Client already disconnected from image server");
            close(this->descriptors.videoFd);
            this->descriptors.videoFd = -1;
            spdlog::warn("Client with id {} disconnected from image server", this->id);
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
