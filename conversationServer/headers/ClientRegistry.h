#pragma once
#include "Client.h"
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include "ServerType.h"
#include <unistd.h>
#include <spdlog/spdlog.h>
class ClientRegistry {
public:
    std::shared_ptr<Client> addClient(std::shared_ptr<Client> client);
    void removeClient(std::shared_ptr<Client> client);
    void disconnectClient(std::size_t id, ServerType type);
    void forEachClient(const std::function<void(const std::shared_ptr<Client>&)>& func);
private:
    std::shared_mutex clientsMutex;
    std::unordered_map<std::size_t, std::shared_ptr<Client>> clients;
};
