#pragma once
#include "ClientRegistry.h"
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>
class SharedContext {
public:
    SharedContext() {
        this->clientRegistry = std::make_shared<ClientRegistry>();
        spdlog::debug("Client Registry created in SharedContext");
    }
    const std::shared_ptr<ClientRegistry>& getClientRegistry() { return this->clientRegistry ? this->clientRegistry : throw std::runtime_error("ClientRegistry is not initialized"); }
private:
    std::shared_ptr<ClientRegistry> clientRegistry = nullptr;

};
