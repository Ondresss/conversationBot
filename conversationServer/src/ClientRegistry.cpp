#include "../headers/ClientRegistry.h"
#include <mutex>
#include <shared_mutex>
#include <spdlog/spdlog.h>

std::shared_ptr<Client> ClientRegistry::addClient(std::shared_ptr<Client> client) {
    spdlog::info("ClientRegistry -> addClient: About to add client to the session registry...");
    std::unique_lock<std::shared_mutex> lock(this->clientsMutex);
    spdlog::debug("ClientRegistry -> addClient: Lock acquired, adding client...");
    auto clientsIt = this->clients.find(client->getId());
    if(clientsIt != this->clients.end()) {
        spdlog::info("ClientRegistry -> addClient: Client with ID {} already exists, updating...", client->getId());
        auto& client_ = *clientsIt->second;
        if(client_.getId() == client->getId()) {
            throw std::runtime_error("Client with same ID already exists");
        }
        if(client->getDescriptors().videoFd != client_.getDescriptors().videoFd && client->getDescriptors().videoFd != -1) {
            spdlog::debug("ClientRegistry -> addClient: Updating video server descriptor for client with ID {}", client->getId());
            client_.setImageServerDescriptor(client->getDescriptors().videoFd);
        }
        if(client->getDescriptors().audioFd != client_.getDescriptors().audioFd && client->getDescriptors().audioFd != -1) {
            spdlog::debug("ClientRegistry -> addClient: Updating audio server descriptor for client with ID {}", client->getId());
            client_.setAudioServerDescriptor(client->getDescriptors().audioFd);
        }
        spdlog::info("ClientRegistry addClient: client updated with ID {}", client->getId());
        return clientsIt->second;
    }

    spdlog::debug("ClientRegistry -> addClient: Client not found, adding client with ID {}", client->getId());
    this->clients[client->getId()] = client;
    spdlog::info("ClientRegistry -> addClient: client added with ID {}", client->getId());
    return this->clients[client->getId()];
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
