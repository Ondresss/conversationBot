//
// Created by andrew on 4/18/26.
//

#include  "../headers/Client.h"
#include "../headers/ServerType.h"
#include <chrono>
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
    switch (type) {
        case ServerType::Conversation:
            if(this->descriptors.audioFd == -1) {
                spdlog::warn("Client with id {} disconnected from audio server", this->id);
                break;
            }
            this->descriptors.audioFd = -1;
            spdlog::warn("Client with id {} disconnected from audio server", this->id);
            break;
        case ServerType::Image:
            if(this->descriptors.videoFd == -1) {
                spdlog::warn("Client with id {} disconnected from image server", this->id);
                break;
            }
            close(this->descriptors.videoFd);
            this->descriptors.videoFd = -1;
            spdlog::warn("Client with id {} disconnected from image server", this->id);
            break;
        default:
            throw std::runtime_error("Invalid server type");
    }
    if(this->descriptors.audioFd == -1 && this->descriptors.videoFd == -1) {
        this->isConnected = false;
    }
}

std::string Client::getUptime() const {
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - this->startTime).count();
    auto chronoUs = std::chrono::microseconds(us);
    auto sec = std::chrono::floor<std::chrono::seconds>(chronoUs);
    std::string uptime = std::format("{:%Q%q}", sec);
    return uptime;
}

nlohmann::json Client::serialize() {
    nlohmann::json j;
    j["id"] = std::to_string(this->id);
    j["ip"] = this->clientIP;
    j["connected"] = this->isConnected;
    j["activeConversationSession"] = this->hasActiveSession();
    j["uptime"] = this->getUptime();
    j["connectedStreams"] = {
        {"audioStream", this->descriptors.audioFd != -1},
        {"videoStream", this->descriptors.videoFd != -1},
    };

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
