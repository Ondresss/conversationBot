//
// Created by andrew on 4/18/26.
//
#pragma once
#include "ConversationSession.h"
#include "ServerType.h"
#include <memory>
#include <shared_mutex>
#include <string>
#include <nlohmann/json.hpp>
#include <mutex>
#include <unistd.h>
class Client {
public:
    struct Descriptor {
       int audioFd = -1;
       int videoFd = -1;
    };
    struct ClientHistory {
        std::string question;
        std::string answer;
    };
    Client(Descriptor descriptors,const std::string& ip,int port)
    : descriptors(std::move(descriptors)), clientIP(ip), port(port) {
        this->id = 0x0;
    }
    Client() = default;
    [[nodiscard]] const std::string& getIP() const { return clientIP; }
    [[nodiscard]] std::size_t getId() const { return id; }
    [[nodiscard]] int getPort() const { return port; }
    [[nodiscard]] bool getIsConnected() const { return isConnected; }
    [[nodiscard]] const Descriptor& getDescriptors() const { return descriptors; }
    [[nodiscard]] const std::unique_ptr<ConversationSession>& getSession() const { return session; }

    void initializeSession(int secs);
    [[nodiscard]] bool hasInitializedSession() const { return session != nullptr; }
    [[nodiscard]] bool hasActiveSession() const { return session != nullptr && session->isSessionActive(); }
    void refreshSession() { if (session) session->refreshSession(); }

    void disconnect(ServerType type);
    void setIP(const std::string& ip) { this->clientIP = ip; }
    void setPort(int port_) { this->port = port_; }
    void setID(uint64_t id_) { this->id = id_; }
    void setImageServerDescriptor(int fd) { descriptors.videoFd = fd; }
    void setAudioServerDescriptor(int fd) { descriptors.audioFd = fd; }

    inline void addHistoryEntry(const std::string& question,const std::string& answer) {
        this->historyList.push_back(ClientHistory{question,answer});
    }

    nlohmann::json serialize();
private:
    std::shared_mutex mutex;
    Descriptor descriptors{};
    std::string clientIP;
    int port = -1;
    std::size_t id = 0;
    std::vector<ClientHistory> historyList;
    bool isConnected = true;
    std::unique_ptr<ConversationSession> session = nullptr;
};
