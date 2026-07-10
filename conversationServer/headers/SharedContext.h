#pragma once
#include "ClientRegistry.h"
#include <memory>
#include <vector>
class SharedContext {
public:
    SharedContext() {
        this->clientRegistry = std::make_shared<ClientRegistry>();
    }
    const std::shared_ptr<ClientRegistry>& getClientRegistry() { return clientRegistry; }
private:
    std::shared_ptr<ClientRegistry> clientRegistry = nullptr;

};
