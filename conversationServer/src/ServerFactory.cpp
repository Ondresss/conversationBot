#include "../headers/ServerFactory.h"
#include <fstream>
#include <sstream>

std::vector<std::shared_ptr<AbstractServer>> ServerFactory::createActiveServers() {
    std::ifstream configFile(this->configPath);
    std::stringstream ss;
    ss << configFile.rdbuf();
    nlohmann::json config = nlohmann::json::parse(ss.str());
    std::vector<std::shared_ptr<AbstractServer>> activeServers;
    if(config.contains("imageServer") && config["imageServer"]["active"]) {
        activeServers.push_back(ImageServer::loadFromConfig(this->configPath));
    } else {
        spdlog::warn("ServerFactory -> createActiveServers: No image server configured or active.");
    }
    activeServers.push_back(ConversationServer::loadFromConfig(this->configPath));
    return activeServers;
}
