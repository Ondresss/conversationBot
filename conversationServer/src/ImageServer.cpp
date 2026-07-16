#include "../headers/ImageServer.h"
#include <memory>
#include <opencv2/core/mat.hpp>
#include <spdlog/spdlog.h>
#include <sys/types.h>
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

    if(json.contains("imageServer")) {
        if(json["imageServer"].contains("period")) {
            params.period = json["imageServer"].value("period",-1);
        }
        if(json["imageServer"].contains("noBufferedImages")) {
            params.noBufferedImages = json["imageServer"].value("noBufferedImages", -1);
        }
        if(json["imageServer"].contains("compressFormat")) {
            params.compressFormat = json["imageServer"].value("compressFormat", "none");
        }
        if(json["imageServer"].contains("ip")) {
            serverInfo.ip = json["imageServer"].value("ip", "");
        }
        if(json["imageServer"].contains("port")) {
            serverInfo.port = json["imageServer"].value("port", -1);
        }

    } else {
        throw std::runtime_error("ImageServer::loadFromConfig(): No ImageServer section in config");
    }
    serverInfo.type = ServerType::Image;
    return std::make_shared<ImageServer>(serverInfo, params);
}

void ImageServer::run() {
    try {
        if (!this->serverSocket) throw std::runtime_error("ImageServer::run(): serverSocket is null");
        while (true) {
            spdlog::info("Image server: Waiting for new client....");
            auto client = this->serverSocket->waitForConnection();
            this->authenticateClient(client);
            auto updatedClient = this->updateClientRegistry(client);
            spdlog::info("Image server: New client connected with IP {}",updatedClient->getIP());
            this->clientThreads.emplace_back(&ImageServer::handleClient, this, updatedClient);
        }
        } catch (std::exception& e){
            spdlog::error("ImageServer::run() -> Application failed: " + std::string(e.what()));
            std::exit(EXIT_FAILURE);
        }

}



void ImageServer::sendHeaderTCP(std::shared_ptr<Client> client, ServerImageControlHeader header) {
    ssize_t totalHeaderBytes = sizeof(header);
    ssize_t bytesLeft = totalHeaderBytes;
    char* headerPtr = reinterpret_cast<char*>(&header);
    while (bytesLeft > 0) {
        ssize_t sent = send(client->getDescriptors().videoFd, headerPtr, bytesLeft, 0);
        if (sent == -1) throw std::runtime_error("ImageServer::sendHeaderTCP(): send() failed");
        bytesLeft -= sent;
        headerPtr += sent;
    }
}

void ImageServer::handleClient(std::shared_ptr<Client> client) {
    auto clientRegistry = this->context->getClientRegistry();
    try {
        spdlog::info("Image server: Handling client with IP {}", client->getIP());
        ServerImageControlHeader header{.status = ServerImageStatus::INFO, .periodMs = this->params.period, .imageCount = this->params.noBufferedImages, .compressType = "JPEG"};
        this->sendHeaderTCP(client, header);
        std::vector<cv::Mat> bufferedFrames(this->params.noBufferedImages);
        while (true) {
            for (std::size_t i{0}; i < this->params.noBufferedImages; ++i) {
                this->recieveImageTCP(client, bufferedFrames[i]);
                spdlog::info("Image server: Received image {}", i);
            }
            spdlog::info("Image server: total {} images recieved and buffered",this->params.noBufferedImages);
        }
    } catch (std::exception& e) {
        spdlog::error("ImageServer run: " + std::string(e.what()));
    }
    client->disconnect(ServerType::Image);
    clientRegistry->removeClient(client);
}


void ImageServer::recvHeaderTCP(std::shared_ptr<Client> client, ClientImageHeader& header) {
    ssize_t totalHeaderBytes = sizeof(header);
    ssize_t bytesLeft = totalHeaderBytes;
    char* headerPtr = reinterpret_cast<char*>(&header);
    while (bytesLeft > 0) {
        ssize_t recvBytes = recv(client->getDescriptors().videoFd, headerPtr, bytesLeft, 0);
        if (recvBytes == -1) throw std::runtime_error("ImageServer::recvHeaderTCP(): recv() failed");
        bytesLeft -= recvBytes;
        headerPtr += recvBytes;
    }
}

void ImageServer::recieveImageTCP(std::shared_ptr<Client> client, cv::Mat& image) {
    ClientImageHeader header;
    this->recvHeaderTCP(client, header);
    ssize_t totalImageBytes = header.totalBytes;
    ssize_t bytesLeft = totalImageBytes;
    char* imagePtr = new char[totalImageBytes];
    while (bytesLeft > 0) {
        ssize_t recvBytes = recv(client->getDescriptors().videoFd, imagePtr, bytesLeft, 0);
        if (recvBytes == -1) throw std::runtime_error("ImageServer::recieveImageTCP(): recv() failed");
        bytesLeft -= recvBytes;
        imagePtr += recvBytes;
    }
    image = cv::Mat(header.height, header.width, CV_8UC3, imagePtr);
    delete[] imagePtr;
}
