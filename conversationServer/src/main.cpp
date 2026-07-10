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
#include <spdlog/spdlog.h>
int main(int argc,const char** argv) {
    try {

        std::shared_ptr<SharedContext> context = std::make_shared<SharedContext>();
        ConversationServer::initLogging();
        std::shared_ptr<ConversationServer> server = ConversationServer::loadFromConfig("../server_config.json");
        server->setSharedContext(context);
        std::thread logicThread([&]() {
            server->run();
        });
        logicThread.detach();
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
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    std::exit(EXIT_SUCCESS);
}
