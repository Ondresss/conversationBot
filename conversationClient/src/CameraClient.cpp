#include "../headers/CameraClient.h"
#include <cstddef>
#include <cstdint>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <sys/types.h>
CameraClient::CameraClient(ServerInfo info, CameraHandler::CameraHandlerParams params) : AbstractClient(info), cameraHandler(std::make_unique<CameraHandler>(params)) {}

CameraClient::~CameraClient() {}

void CameraClient::recieveServerImageControlHeaderTCP(ServerImageControlHeader& header) const {
    ssize_t totalBytes = sizeof(ServerImageControlHeader);
    ssize_t bytesRead = 0;
    char* buffer = reinterpret_cast<char*>(&header);
    while(totalBytes > 0) {
        ssize_t currentSentBytes = recv(this->fd, buffer + bytesRead, totalBytes - bytesRead, 0);
        if(currentSentBytes == -1) throw std::runtime_error("CameraClient: recv() failed");
        if(currentSentBytes == 0) throw std::runtime_error("CameraClient: recv() returned 0");
        bytesRead += currentSentBytes;
        totalBytes -= currentSentBytes;
    }
    spdlog::info("CameraClient -> recieveServerImageControlHeader: Recieved Server control header with [status={}, periodMs={}, imageCount={}, compressType={}]",
        toStringServerStatus(header.status), header.periodMs, header.imageCount, header.compressType);
}

void CameraClient::run() {
    try {
        if(!this->cameraHandler) throw std::runtime_error("CameraClient: cameraHandler is null");
        this->connectToServer();
        spdlog::info("Camera client running...");
        ServerImageControlHeader header;
        std::vector<std::vector<uint8_t>> imagesBuffer;
        while(true) {
            this->recieveServerImageControlHeaderTCP(header);
            if(header.periodMs == -1) throw std::runtime_error("CameraClient: periodMs is -1");
            if(header.imageCount == -1) throw std::runtime_error("CameraClient: imageCount is -1");
            if(header.status == ServerImageStatus::ERROR) throw std::runtime_error("CameraClient: status is ERROR");
            std::this_thread::sleep_for(
                           std::chrono::milliseconds(header.periodMs));
            imagesBuffer.reserve(header.imageCount);
            for(std::size_t i{0}; i < header.imageCount; ++i) {
                auto image = this->cameraHandler->captureImage();
                if(!image.has_value()) throw std::runtime_error("CameraClient: image is null");
                imagesBuffer.emplace_back(image.value().begin(), image.value().end());
            }
            this->sendImagesTCP(imagesBuffer);
            imagesBuffer.clear();
        }

    } catch (const std::exception& e) {
        spdlog::error("CameraClient -> run(): Ended with error {}", e.what());
    }
    this->disconnectFromServer();
    spdlog::warn("Camera client ended connection!");
}

void CameraClient::sendClientImageHeaderTCP(const ClientImageHeader& header) const {
    spdlog::debug("CameraClient -> sending client image header with totalBytes={}", header.totalBytes);
    const char* headerData = reinterpret_cast<const char*>(&header);
    ssize_t headerBytesToSend = sizeof(ClientImageHeader);
    while(headerBytesToSend > 0) {
        ssize_t sentBytes = send(this->fd,
                                headerData + (sizeof(ClientImageHeader) - headerBytesToSend),
                                headerBytesToSend,
                                0);
        if(sentBytes <= 0) {
            throw std::runtime_error("CameraClient: send header failed (server disconnected or error)");
        }
        headerBytesToSend -= sentBytes;
    }
    spdlog::debug("CameraClient -> sent client image header successfully");
}

void CameraClient::sendImagesTCP(const std::vector<std::vector<uint8_t>>& imagesBuffer) const {
    for(const auto& image : imagesBuffer) {
        auto params = this->cameraHandler->getParams();
        ClientImageHeader header;
        header.totalBytes = image.size();
        header.width = params.width;
        header.height = params.height;
        this->sendClientImageHeaderTCP(header);
        ssize_t bytesToSend = header.totalBytes;
        while(bytesToSend > 0) {
            ssize_t sentBytes = send(this->fd, image.data() + (header.totalBytes - bytesToSend), bytesToSend, 0);
            if(sentBytes < 0) throw std::runtime_error("CameraClient: send failed");
            if(!sentBytes) {
                spdlog::warn("CameraClient -> sendImagesTCP: send() -> sent 0 bytes!");
                break;
            }
            bytesToSend -= sentBytes;
        }
        spdlog::debug("CameraClient -> sent image with params [width={}, height={}, totalBytes={}]", params.width, params.height, header.totalBytes);
    }
}
