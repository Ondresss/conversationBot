#pragma once

#include <memory>
#include <stop_token>
#include <thread>
#include <vector>
#include "AbstractServer.h"
#include <csignal>
class ServerManager {
public:
    ~ServerManager();
    void runAll();
    void stopAll();
    void setSharedContextAll(std::shared_ptr<SharedContext> context);
    void setServers(const std::vector<std::shared_ptr<AbstractServer>>& servers) { this->servers = servers; }
    static ServerManager& getInstance();
    static void runAndWait();
private:
    ServerManager();
    ServerManager(const std::vector<std::shared_ptr<AbstractServer>>& servers) : servers(servers) {}
    std::vector<std::shared_ptr<AbstractServer>> servers;
    std::vector<std::jthread> threads;
};
