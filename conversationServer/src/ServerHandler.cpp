//
// Created by andrew on 4/18/26.
//
#include "../headers/ServerHandler.h"
#include <spdlog/spdlog.h>

void ServerHandler::setupRestRoutes() {
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/getClients",Pistache::Rest::Routes::bind(&ServerHandler::getClients, this));
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/disconnectClient",Pistache::Rest::Routes::bind(&ServerHandler::disconnectClient, this));
    spdlog::info("All routes setup\n");
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

        std::string idRaw = params.get("id").value();

        idRaw.erase(std::remove(idRaw.begin(), idRaw.end(), '\"'), idRaw.end());

        uint64_t targetId = std::stoull(idRaw);

        bool found = false;
        auto clients = this->server->getClients();

        for (auto& c : clients) {
            if (c->getId() == targetId) {
                found = true;
                c->setDisconnected();
                break;
            }
        }

        if (found) {
            response.send(Pistache::Http::Code::Ok, "Client disconnected");
        } else {
            response.send(Pistache::Http::Code::Not_Found, "Client not found or already offline");
        }
    } catch (const std::exception& e) {
        spdlog::error("Disconnect error: {}", e.what());
        response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
    }
}

void ServerHandler::getClients(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    auto logger = ClientLogger::getInstance();
    auto clientList = logger.selectAll();
    nlohmann::json jsonList = nlohmann::json::array();
    for (auto& c : clientList) {
        jsonList.push_back(c.serialize());
    }
    response.headers().add<Pistache::Http::Header::ContentType>(MIME(Application, Json));
    response.send(Pistache::Http::Code::Ok, jsonList.dump());
}
