//
// Created by andrew on 4/18/26.
//
#include "../headers/ServerHandler.h"
#include <cstddef>
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

void ServerHandler::setupRestRoutes() {
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/getClients",Pistache::Rest::Routes::bind(&ServerHandler::getClientsAll, this));
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/getClientsActive",Pistache::Rest::Routes::bind(&ServerHandler::getClientsActiveAll, this));
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/disconnectClient",Pistache::Rest::Routes::bind(&ServerHandler::disconnectClient, this));
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/getClientImage",Pistache::Rest::Routes::bind(&ServerHandler::getClientImage, this));
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/getClientImageAnalysis", Pistache::Rest::Routes::bind(&ServerHandler::getClientImageAnalysis, this));
    Pistache::Rest::Routes::Get(this->router, "/conversationServer/getClientProcessedImage", Pistache::Rest::Routes::bind(&ServerHandler::getClientProcessedImage, this));
    spdlog::info("All routes setup\n");
}

void ServerHandler::getClientImageAnalysis(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    try {
        auto params = request.query();
        if(!params.has("clientId")) {
            response.send(Pistache::Http::Code::Bad_Request, "Missing clientId");
            return;
        }
        auto clientId = params.get("clientId").value();
        std::size_t cId = std::stoul(clientId);
        auto serverImageCache = this->context->getImageServerContext();
        auto analysis = serverImageCache->getClientsAnalysis(cId);
        if(!analysis) {
            spdlog::error("No analysis found for client {}", cId);
            response.send(Pistache::Http::Code::Not_Found, "No analysis found for client");
            return;
        }
        nlohmann::json jsonResponse = analysis->serialize();
        response.send(Pistache::Http::Code::Ok, jsonResponse.dump());
    } catch (const std::exception& e) {
        spdlog::error("Error in getClientImageAnalysis: {}", e.what());
        response.send(Pistache::Http::Code::Internal_Server_Error, "Internal Server Error");
    }
}

void ServerHandler::getClientProcessedImage(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    try {
        auto params = request.query();
        if(!params.has("clientId")) {
            response.send(Pistache::Http::Code::Bad_Request, "Missing clientId");
            return;
        }
        auto clientId = params.get("clientId").value();
        std::size_t cId = std::stoul(clientId);
        auto imageCache = this->context->getImageServerContext();
        auto imageAnalysis = imageCache->getClientsAnalysis(cId);
        if(!imageAnalysis) {
            response.send(Pistache::Http::Code::Not_Found, "No analysis found for client");
            return;
        }
        auto processedImage = imageAnalysis->processedImage;
        if (processedImage.empty()) {
            response.send(Pistache::Http::Code::Not_Found, "Image not found");
            return;
        }
        std::vector<uchar> jpegBuffer;
        bool success = cv::imencode(".jpg", processedImage, jpegBuffer);

        if (!success || jpegBuffer.empty()) {
            spdlog::error("REST: Failed to encode processedImage to JPEG for client {}", clientId);
            response.send(Pistache::Http::Code::Internal_Server_Error, "Failed to encode image");
            return;
        }

        spdlog::info("REST: client JPEG encoded size = {} bytes", jpegBuffer.size());
        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Image, Jpeg));
        response.send(
            Pistache::Http::Code::Ok,
            reinterpret_cast<const char*>(jpegBuffer.data()),
            jpegBuffer.size()
        );

    } catch (const std::exception& e) {
        spdlog::error("Error in getClientProcessedImage: {}", e.what());
        response.send(Pistache::Http::Code::Internal_Server_Error, "Internal Server Error");
    }
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

void ServerHandler::getClientImage(const Pistache::Rest::Request& request, Pistache::Http::ResponseWriter response) {
    try {
        auto params = request.query();
        if (!params.has("id")) {
            response.send(Pistache::Http::Code::Bad_Request, "Missing id");
            return;
        }
        if(!params.has("imageIndex")) {
            response.send(Pistache::Http::Code::Bad_Request, "Missing imageIndex");
            return;
        }
        int imageIndex = std::stoi(params.get("imageIndex").value());
        std::size_t clientId = std::stoull(params.get("id").value());
        auto registry = this->context->getClientRegistry();
        cv::Mat clientImage;
        auto imageServerContext = this->context->getImageServerContext();
        auto analysis = imageServerContext->getClientsAnalysis(clientId);
        if (analysis == nullptr) {
            response.send(Pistache::Http::Code::Not_Found, "Image not found");
            return;
        }
        clientImage = analysis->originalImage;
        if (clientImage.empty()) {
            response.send(Pistache::Http::Code::Not_Found, "Image not found");
            return;
        }
        spdlog::info("REST: found client image for client {}", clientId);
        size_t dataSize = clientImage.total() * clientImage.elemSize();
        spdlog::info("REST: client image size = {}", dataSize);

        response.headers().add<Pistache::Http::Header::ContentType>(MIME(Image, Jpeg));

        response.send(
            Pistache::Http::Code::Ok,
            reinterpret_cast<const char*>(clientImage.data),
            dataSize
        );
    } catch (const std::exception& e) {
        spdlog::error("Get client image error: {}", e.what());
        response.send(Pistache::Http::Code::Internal_Server_Error, e.what());
    }
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
