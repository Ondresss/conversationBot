#pragma once

#include "AbstractClient.h"
#include "CameraHandler.h"
#include "ServerImageControlHeader.h"
#include "ServerInfo.h"
#include <memory>
#include <thread>
#include <chrono>
#include <optional>
#include "ClientImageHeader.h"
#include <sys/socket.h>
class CameraClient : public AbstractClient {
public:
    CameraClient(ServerInfo info,CameraHandler::CameraHandlerParams params);
    ~CameraClient();
    void run() override;
private:
   void recieveServerImageControlHeaderTCP(ServerImageControlHeader& header) const;
   void sendImagesTCP(const std::vector<std::vector<uint8_t>>& imagesBuffer) const;
   void sendClientImageHeaderTCP(const ClientImageHeader& header) const;
   std::unique_ptr<CameraHandler> cameraHandler = nullptr;

};
