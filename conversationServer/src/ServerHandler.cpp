//
// Created by andrew on 4/18/26.
//
#include "../headers/ServerHandler.h"
#include <spdlog/spdlog.h>

void ServerHandler::setupRestRoutes() {
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/getClients",Pistache::Rest::Routes::bind(&ServerHandler::getClients, this));
    spdlog::info("All routes setup\n");
}
void ServerHandler::setupCors(Pistache::Http::ResponseWriter& response) {
    auto headers = response.headers();
    headers.add<Pistache::Http::Header::AccessControlAllowOrigin>("*");
    headers.add<Pistache::Http::Header::AccessControlAllowMethods>("GET, POST, OPTIONS, PUT, DELETE");
    headers.add<Pistache::Http::Header::AccessControlAllowHeaders>("Origin, X-Requested-With, Content-Type, Accept, Authorization");
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
