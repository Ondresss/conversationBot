#include "../headers/ImageServer.h"
#include <memory>
ImageServer::ImageServer(ServerInfo serverInfo, ImageServerParams params, std::shared_ptr<SharedContext> context = nullptr) : AbstractServer(serverInfo, context), params(std::move(params)) {}

std::shared_ptr<ImageServer> ImageServer::loadFromConfig(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        spdlog::error("ImageServer::loadfromConfig(): Could not open file {}", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    nlohmann::json json = nlohmann::json::parse(ss.str());
    ImageServerParams params{};
    ServerInfo serverInfo{};
    if(json.contains("info")) {
        if(json["info"].contains("port")) {
            serverInfo.port = json["info"].value("port", -1);
        }
        if(json["info"].contains("ip")) {
            serverInfo.ip = json["info"].value("ip", "");
        }
    }
    if(json.contains("ImageServer")) {
        if(json["ImageServer"].contains("period")) {
            params.period = json["ImageServer"].value("period",-1);
        }
        if(json["ImageServer"].contains("noBufferedImages")) {
            params.noBufferedImages = json["ImageServer"].value("noBufferedImages", -1);
        }
        if(json["ImageServer"].contains("compressFormat")) {
            params.compressFormat = json["ImageServer"].value("compressFormat", "none");
        }
    } else {
        throw std::runtime_error("ImageServer::loadFromConfig(): No ImageServer section in config");
    }
    serverInfo.type = ServerType::Image;
    return std::make_shared<ImageServer>(serverInfo, params);
}

void ImageServer::run() {
}
