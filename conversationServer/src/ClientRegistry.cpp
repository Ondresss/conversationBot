#include "../headers/ClientRegistry.h"
#include <mutex>
#include <shared_mutex>
#include <spdlog/spdlog.h>

void ClientRegistry::addClient(std::shared_ptr<Client> client) {
    std::unique_lock<std::shared_mutex> lock(this->clientsMutex);
    auto clientsIt = this->clients.find(client->getId());
    if(clientsIt != this->clients.end()) {
        auto& client_ = *clientsIt->second;
        if(client_.getId() == client->getId()) {
            throw std::runtime_error("Client with same ID already exists");
        }
        if(client->getDescriptors().videoFd != client_.getDescriptors().videoFd && client->getDescriptors().videoFd != -1) {
            client_.setImageServerDescriptor(client->getDescriptors().videoFd);
        }
        if(client->getDescriptors().audioFd != client_.getDescriptors().audioFd && client->getDescriptors().audioFd != -1) {
            client_.setAudioServerDescriptor(client->getDescriptors().audioFd);
        }
        spdlog::info("ClientRegistry addClient: client updated with ID {}", client->getId());
        return;
    }
    this->clients[client->getId()] = std::move(client);
    spdlog::info("ClientRegistry addClient: client added with ID {}", client->getId());
}

void ClientRegistry::removeClient(std::shared_ptr<Client> client) {
    std::unique_lock<std::shared_mutex> lock(this->clientsMutex);
    try {
        auto client_ = this->clients.at(client->getId());
        if(client_->getDescriptors().audioFd == -1 && client_->getDescriptors().videoFd == -1) {
            this->clients.erase(client->getId());
        } else {
            spdlog::warn("ClientRegistry removeClient: client still has active descriptors");
        }
        spdlog::info("ClientRegistry removeClient: client removed with ID {}", client->getId());
    } catch(const std::out_of_range&) {
        spdlog::warn("ClientRegistry removeClient: client not found");
        return;
    }
}

void ClientRegistry::forEachClient(const std::function<void(const std::shared_ptr<Client>&)>& func) {
    std::shared_lock<std::shared_mutex> lock(this->clientsMutex);
    for(const auto& [id, client] : this->clients) {
        func(client);
    }
}

void ClientRegistry::disconnectClient(std::size_t id, ServerType type) {
    std::unique_lock<std::shared_mutex> lock(this->clientsMutex);
    auto clientsIt = this->clients.find(id);
    if(clientsIt != this->clients.end()) {
        auto& client = *clientsIt->second;
        client.disconnect(type);
        this->clients.erase(clientsIt);
        spdlog::info("ClientRegistry disconnectClient: client disconnected with ID {} with type {}", id,toStringServerType(type));
    } else {
        throw std::runtime_error("Client not found for disconnection");
    }
}
