#pragma once
#include "Client.h"
#include "ServerInfo.h"
#include "ServerSocket.h"
#include <memory>
#include "SharedContext.h"
class AbstractServer {
public:
    AbstractServer(ServerInfo serverInfo, std::shared_ptr<SharedContext> context) : serverSocket(std::make_unique<ServerSocket>(std::move(serverInfo))), context(std::move(context)) {};
    virtual ~AbstractServer() = default;
    virtual void run() = 0;
    virtual void updateClientRegistry(std::shared_ptr<Client> client) {
        auto clientRegistry = context->getClientRegistry();
        clientRegistry->addClient(client);
    }
    virtual void setSharedContext(std::shared_ptr<SharedContext> context) {
        this->context = std::move(context);
    }
protected:
    std::unique_ptr<ServerSocket> serverSocket = nullptr;
    std::shared_ptr<SharedContext> context = nullptr;
};
