#pragma once
#include "Client.h"
#include "ServerInfo.h"
#include "ServerSocket.h"
#include <compare>
#include <memory>
#include <opencv2/photo/ccm.hpp>
#include <thread>
#include <vector>
#include "SharedContext.h"
#include "ClientAuthHeader.h"
#include <sys/types.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>
#include "ServerAuthResponseHeader.h"

class AbstractServer {
public:
    AbstractServer(ServerInfo serverInfo, std::shared_ptr<SharedContext> context) : serverSocket(std::make_unique<ServerSocket>(std::move(serverInfo))), context(std::move(context)) {
        spdlog::debug("AbstractServer() constructor ran");
    };
    virtual ~AbstractServer() {
        for (auto& thread : clientThreads) {
            if (thread.joinable()) thread.join();
        }
    }
    virtual void run() = 0;
    virtual void handleClient(std::shared_ptr<Client> client) = 0;
    void authenticateClient(std::shared_ptr<Client> client);
    void sendAuthResponse(std::shared_ptr<Client> client, ServerAuthStatus status);

    std::shared_ptr<Client> updateClientRegistry(std::shared_ptr<Client> client) {
        spdlog::info("AbstractServer -> updateClientRegistry: About to update client {} in the registry...", client->getId());
        auto clientRegistry = context->getClientRegistry();
        if(!clientRegistry) {
            spdlog::error("AbstractServer -> updateClientRegistry: Client registry is not available.");
            throw std::runtime_error("Client registry is not available. = is nullptr");
        }
        return clientRegistry->addClient(client);
    }
    virtual void setSharedContext(std::shared_ptr<SharedContext> context) {
        this->context = std::move(context);
    }

    static void initLogging();
protected:
    std::unique_ptr<ServerSocket> serverSocket = nullptr;
    std::shared_ptr<SharedContext> context = nullptr;
    std::vector<std::thread> clientThreads;
};
