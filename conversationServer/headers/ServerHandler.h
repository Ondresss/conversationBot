//
// Created by andrew on 4/18/26.
//
#pragma once
#include <memory>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include  "ConversationServer.h"
#include "SharedContext.h"
class ServerHandler : public Pistache::Http::Handler {
public:

    std::shared_ptr<Pistache::Tcp::Handler> clone() const override {
        return std::make_shared<ServerHandler>(this->server, this->context);
    }
    ServerHandler(std::shared_ptr<ConversationServer> server_,std::shared_ptr<SharedContext> context_) : server(std::move(server_)), context(std::move(context_)) {
        this->setupRestRoutes();
        spdlog::debug("ServerHandler created");
    }
    void onRequest(const Pistache::Http::Request& request, Pistache::Http::ResponseWriter response) override {};
    void setupRestRoutes();
    void setupCors(Pistache::Http::ResponseWriter& response);

    void getClientsAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void getClientsActiveAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);
    void disconnectClient(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response);

    std::shared_ptr<Pistache::Rest::Router> getRouter() {
        return std::make_shared<Pistache::Rest::Router>(router);
    }
private:
    Pistache::Rest::Router router;
    std::shared_ptr<ConversationServer> server;
    std::shared_ptr<SharedContext> context = nullptr;
};
