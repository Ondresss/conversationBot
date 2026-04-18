//
// Created by andrew on 4/18/26.
//
#pragma once
#include <string>
#include <nlohmann/json.hpp>
class Client {
public:
    struct ClientHistory {
        std::string question;
        std::string answer;
    };
    Client(int clientFd,const std::string& ip,int port)
    : clientFd(clientFd), clientIP(ip), port(port) {
        this->id = 0x0;
    }
    Client() = default;
    [[nodiscard]] int getFd() const { return clientFd; }
    [[nodiscard]] const std::string& getIP() const { return clientIP; }
    [[nodiscard]] std::size_t getId() const { return id; }
    [[nodiscard]] int getPort() const { return port; }
    [[nodiscard]] bool getIsConnected() const { return isConnected; }

    void setDisconnected() { isConnected = false; }
    void setIP(const std::string& ip) { this->clientIP = ip; }
    void setFd(int clientFd_) { this->clientFd = clientFd_; }
    void setPort(int port_) { this->port = port_; }
    void setID(uint64_t id_) { this->id = id_; }

    inline void addHistoryEntry(const std::string& question,const std::string& answer) {
        this->historyList.push_back(ClientHistory{question,answer});
    }

    nlohmann::json serialize();
private:
    int clientFd = -1;
    std::string clientIP;
    int port = -1;
    std::size_t id = 0;
    std::vector<ClientHistory> historyList;
    bool isConnected = true;
};