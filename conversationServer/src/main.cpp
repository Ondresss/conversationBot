//
// Created by andrew on 05.03.26.
//

#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
#include "../headers/ConversationServer.h"
#include <pistache/net.h>
#include "../headers/ServerHandler.h"
#include "../headers/ImageServer.h"
#include <spdlog/spdlog.h>
#include <vector>
#include <thread>
#include <memory>
#include "../headers/ServerManager.h"
int main(int argc,const char** argv) {
    try {

        std::shared_ptr<SharedContext> context = std::make_shared<SharedContext>();
        AbstractServer::initLogging();
        std::shared_ptr<ConversationServer> server = ConversationServer::loadFromConfig("../server_config.json");
        std::shared_ptr<ImageServer> imageServer = ImageServer::loadFromConfig("../server_config.json");
        std::vector<std::shared_ptr<AbstractServer>> servers {server, imageServer};
        std::unique_ptr<ServerManager> serverManager = std::make_unique<ServerManager>(servers);
        serverManager->runAll();
        Pistache::Address addr(Pistache::Ipv4::any(), Pistache::Port(8081));
        auto opts = Pistache::Http::Endpoint::options().threads(1);
        auto service = std::make_shared<ServerHandler>(server, context);
        Pistache::Http::Endpoint webServer(addr);
        webServer.init(opts);
        webServer.setHandler(service->getRouter()->handler());
        spdlog::info("Web server started on port {} ",8081);
        webServer.serve();
        spdlog::warn("Web server stopped ");

    } catch (std::exception& e) {
        spdlog::error("Main() -> Application failed: " + std::string(e.what()));
        std::exit(EXIT_FAILURE);
    }
    std::exit(EXIT_SUCCESS);
}
