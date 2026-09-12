#pragma once
#include "ClientRegistry.h"
#include <latch>
#include <memory>
#include <optional>
#include <spdlog/spdlog.h>
#include <vector>
#include "ConversationServerCache.h"
#include "ImageServerCache.h"
#include "ServerType.h"

class AbstractServer;
class ImageServer;
class ConversationServer;
class SharedContext {
public:
    SharedContext() : clientRegistry(std::make_shared<ClientRegistry>()), imageServerContext(std::make_shared<ImageServerCache>()), conversationServerContext(std::make_shared<ConversationServerCache>()) {
        spdlog::debug("Client Registry created in SharedContext");
    }
    const std::shared_ptr<ClientRegistry>& getClientRegistry() { return this->clientRegistry ? this->clientRegistry : throw std::runtime_error("ClientRegistry is not initialized"); }
    const std::shared_ptr<ImageServerCache>& getImageServerContext() { return this->imageServerContext ? this->imageServerContext : throw std::runtime_error("ImageServerContext is not initialized"); }
    const std::shared_ptr<ConversationServerCache>& getConversationServerContext() { return this->conversationServerContext ? this->conversationServerContext : throw std::runtime_error("ConversationServerContext is not initialized"); }
    void switchActiveStateForCache(ServerType type, bool state);
private:
    std::shared_ptr<ClientRegistry> clientRegistry = nullptr;
    std::shared_ptr<ImageServerCache> imageServerContext = nullptr;
    std::shared_ptr<ConversationServerCache> conversationServerContext = nullptr;
};
