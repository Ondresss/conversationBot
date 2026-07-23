//
// Created by andrew on 4/18/26.
//
#include "../headers/ServerHandler.h"
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

void ServerHandler::setupRestRoutes() {
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/getClients",Pistache::Rest::Routes::bind(&ServerHandler::getClientsAll, this));
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/getClientsActive",Pistache::Rest::Routes::bind(&ServerHandler::getClientsActiveAll, this));
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/disconnectClient",Pistache::Rest::Routes::bind(&ServerHandler::disconnectClient, this));
    spdlog::info("All routes setup\n");
}

void ServerHandler::getClientsActiveAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    try {
       auto registry = this->context->getClientRegistry();
       nlohmann::json jsonList = nlohmann::json::array();
       registry->forEachClient([&jsonList](const std::shared_ptr<Client>& client) {
           if(client->getIsConnected() && (client->getDescriptors().audioFd != -1 || client->getDescriptors().videoFd != -1)) {
               jsonList.push_back(client->serialize());
           }
       });
       response.send(Pistache::Http::Code::Ok, jsonList.dump());
    } catch (const std::exception& e) {
        spdlog::error("Get clients active error: {}", e.what());
        response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
    }
}

void ServerHandler::setupCors(Pistache::Http::ResponseWriter& response) {
    auto headers = response.headers();
    headers.add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
    headers.add<Pistache::Http::Header::AccessControlAllowMethods>("GET, POST, OPTIONS, PUT, DELETE");
    headers.add<Pistache::Http::Header::AccessControlAllowHeaders>("Origin, X-Requested-With, Content-Type, Accept, Authorization");
}


void ServerHandler::disconnectClient(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    try {
        auto params = request.query();
        if (!params.has("id")) {
            response.send(Pistache::Http::Code::Bad_Request, "Missing id");
            return;
        }
        if(!params.has("ServerType")) {
            response.send(Pistache::Http::Code::Bad_Request, "Missing ServerType");
            return;
        }

        std::string idRaw = params.get("id").value();
        std::string serverType = params.get("ServerType").value();

        try {
            uint64_t targetId = std::stoull(idRaw);
            ServerType type = serverType == "voice" ? ServerType::Conversation : ServerType::Image;
            auto registry = this->context->getClientRegistry();
            spdlog::info("REST Controller: Disconnecting client with ID {} of type {}", targetId, toStringServerType(type));
            registry->disconnectClient(targetId, type);
            spdlog::info("REST Controller: Client disconnected successfully");
            response.send(Pistache::Http::Code::Ok, "Client disconnected");
        } catch (const std::exception& e) {
            response.send(Pistache::Http::Code::Not_Found, "Client not found or already offline " + std::string(e.what()));
        }
    } catch (const std::exception& e) {
        spdlog::error("Disconnect error: {}", e.what());
        response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
    }
}

void ServerHandler::getClientsAll(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    try {
        auto logger = ClientLogger::getInstance();
        auto clientList = logger.selectAll();
        nlohmann::json jsonList = nlohmann::json::array();
        for (auto& c : clientList) {
            jsonList.push_back(c->serialize());
        }
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
        response.send(Pistache::Http::Code::Ok, jsonList.dump());
    } catch (const std::exception& e) {
        spdlog::error("Get clients error: {}", e.what());
        response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
    }
}
