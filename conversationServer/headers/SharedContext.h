#pragma once
#include "ClientRegistry.h"
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>
#include "ImageServerCache.h"

class SharedContext {
public:
    SharedContext() {
        this->clientRegistry = std::make_shared<ClientRegistry>();
        this->imageServerContext = std::make_shared<ImageServerCache>();
        spdlog::debug("Client Registry created in SharedContext");
    }
    const std::shared_ptr<ClientRegistry>& getClientRegistry() { return this->clientRegistry ? this->clientRegistry : throw std::runtime_error("ClientRegistry is not initialized"); }
    const std::shared_ptr<ImageServerCache>& getImageServerContext() { return this->imageServerContext ? this->imageServerContext : throw std::runtime_error("ImageServerContext is not initialized"); }
private:
    std::shared_ptr<ClientRegistry> clientRegistry = nullptr;
    std::shared_ptr<ImageServerCache> imageServerContext = nullptr;

};
