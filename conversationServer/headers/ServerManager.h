#pragma once

#include <memory>
#include <thread>
#include <vector>
#include "AbstractServer.h"

class ServerManager {
public:
    ServerManager(const std::vector<std::shared_ptr<AbstractServer>>& servers) : servers(servers) {}
    ~ServerManager();
    void runAll();
    void setSharedContextAll(std::shared_ptr<SharedContext> context);
private:
    std::vector<std::shared_ptr<AbstractServer>> servers;
    std::vector<std::jthread> threads;
};
