#include "../headers/ClientRegistry.h"
#include <mutex>
#include <shared_mutex>

void ClientRegistry::addClient(std::shared_ptr<Client> client) {
    std::unique_lock<std::shared_mutex> lock(this->clientsMutex);
    auto clientsIt = this->clients.find(client->getId());
    if(clientsIt != this->clients.end()) {
        auto& client_ = *clientsIt->second;
        if(client->getDescriptors().videoFd != client_.getDescriptors().videoFd && client->getDescriptors().videoFd != -1) {
            client_.setImageServerDescriptor(client->getDescriptors().videoFd);
        }
        if(client->getDescriptors().audioFd != client_.getDescriptors().audioFd && client->getDescriptors().audioFd != -1) {
            client_.setAudioServerDescriptor(client->getDescriptors().audioFd);
        }
        return;
    }
    this->clients[client->getId()] = std::move(client);
}

void ClientRegistry::removeClient(std::shared_ptr<Client> client) {
    std::unique_lock<std::shared_mutex> lock(this->clientsMutex);
    this->clients.erase(client->getId());
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
    } else {
        throw std::runtime_error("Client not found for disconnection");
    }
}
