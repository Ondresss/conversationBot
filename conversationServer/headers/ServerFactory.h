#pragma once
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "ImageServer.h"
#include "ConversationServer.h"
#include <spdlog/spdlog.h>
#include "SharedContext.h"
class ServerFactory {
public:
    ServerFactory(const std::string& configPath) : configPath(configPath) {}
    std::vector<std::shared_ptr<AbstractServer>> createActiveServers();
private:
    std::string configPath;
};
