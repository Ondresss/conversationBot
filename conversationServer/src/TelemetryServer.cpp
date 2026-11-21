#include "../headers/TelemetryServer.h"

void TelemetryServer::run(std::stop_token stopToken) {
    try {
        if (!this->serverSocket) throw std::runtime_error("Telemetry::run(): serverSocket is null");
        std::stop_callback stopCb(stopToken, [this]() {
            if (this->serverSocket) {
                this->disconnectAllClients();
                this->serverSocket->shutdown();
            }
        });

        while (!stopToken.stop_requested()) {
            spdlog::info("Telemetry::run(): Waiting for new client....");
            auto client = this->serverSocket->waitForConnection();
            spdlog::info("Telemetry::run(): Client connected but is unauthenticated");
            if(client->getDescriptors().telemetryFd == -1) throw std::runtime_error("Telemetry::run(): Client telemetryFd is -1");
            this->authenticateClient(client);
            auto updatedClient = this->updateClientRegistry(client);
            spdlog::info("Telemetry server: New client connected with IP {}",updatedClient->getIP());
            this->clientThreads.emplace_back(&TelemetryServer::handleClient, this, updatedClient);
        }
    } catch(const std::exception& e) {
        spdlog::error("Telemetry::run() -> TelemetryServer failed: " + std::string(e.what()));
    }
    spdlog::warn("Telemetry Server was stopped");
}


std::vector<TelemetryType> TelemetryServer::recieveAvailableSensorsTCP(const std::shared_ptr<Client>& client) {
    TelemetryAvailabilityHeader header;
    ssize_t headerBytesToReceive = sizeof(TelemetryAvailabilityHeader);
    char* headerData = reinterpret_cast<char*>(&header);
    while(headerBytesToReceive > 0) {
        ssize_t receivedBytes = recv(client->getDescriptors().telemetryFd,
                                    headerData + (sizeof(TelemetryAvailabilityHeader) - headerBytesToReceive),
                                    headerBytesToReceive,
                                    0);
        if(receivedBytes <= 0) {
            throw std::runtime_error("TelemetryClient: receive server status header failed (server disconnected or error)");
        }
        headerBytesToReceive -= receivedBytes;
    }
    return header.decodeSensors(header.noSensors);

}


std::shared_ptr<TelemetryClientData> TelemetryServer::recieveTelemetryDataTCP(const std::shared_ptr<Client>& client) {
    TelemetryClientDataHeader header;
    ssize_t headerBytesToReceive = sizeof(TelemetryClientDataHeader);
    char* headerData = reinterpret_cast<char*>(&header);
    while(headerBytesToReceive > 0) {
        ssize_t receivedBytes = recv(client->getDescriptors().telemetryFd,
                                    headerData + (sizeof(TelemetryClientDataHeader) - headerBytesToReceive),
                                    headerBytesToReceive,
                                    0);
        if(receivedBytes <= 0) {    
            throw std::runtime_error("TelemetryClient: receive telemetry data header failed (server disconnected or error)");
        }
        headerBytesToReceive -= receivedBytes;
    }
    spdlog::debug("TelemetryClient: Received telemetry data header with dataSize={}",
    header.dataSize);
    TelemetryData data;
    ssize_t dataBytesToReceive = header.dataSize;
    char* dataPtr = reinterpret_cast<char*>(&data);
    while(dataBytesToReceive > 0) {
        ssize_t receivedBytes = recv(client->getDescriptors().telemetryFd,
                                    dataPtr + (sizeof(TelemetryData) - dataBytesToReceive),
                                    dataBytesToReceive,
                                    0);
        if(receivedBytes <= 0) {
            throw std::runtime_error("TelemetryClient: receive telemetry data failed (server disconnected or error)");
        }
        dataBytesToReceive -= receivedBytes;
    }
    return std::make_shared<TelemetryClientData>(data);
}


void TelemetryServer::handleClient(std::shared_ptr<Client> client) {
    auto clientRegistry = this->context->getClientRegistry();
    auto telemetryServerContext = this->context->getTelemetryServerContext();
    try {
        std::vector<TelemetryType> availableSensors = this->recieveAvailableSensorsTCP(client);
        if(availableSensors.empty()) {
            throw std::runtime_error("TelemetryServer: No available sensors received from client " + std::to_string(client->getId()));
        } else {
            spdlog::info("TelemetryServer: Available sensors received from client {}: {}", client->getId(), availableSensors.size());
            for(const auto& sensor : availableSensors) {
                spdlog::info("TelemetryServer: Sensor type: {}", toStringTelemetryType(sensor));
            }
        }
        TelemetryServerStatusHeader statusHeader{.status = TelemetryServerStatusType::OK,.type = TelemetryServerControlType::FETCH};
        this->sendTelemetryStatusTCP(client, statusHeader);
        while(true) {
            client->getClientSync().acquireWorkerGate();
            ScopeLatchGuard latchGuard(*client);
            this->sendTelemetryStatusTCP(client, statusHeader);
            auto telemetryData = this->recieveTelemetryDataTCP(client);
            spdlog::info("TelemetryServer: Received telemetry data from client {}: timestamp={}, data: \n", client->getId(), telemetryData->toString());
            telemetryServerContext->updateTelemetryData(client, telemetryData);
        }
    } catch(const std::exception& e) {
        spdlog::error("Telemetry::run() -> TelemetryServer failed: " + std::string(e.what()));
    }
    client->disconnect(ServerType::Telemetry);
    clientRegistry->removeClient(client);
}

void TelemetryServer::sendTelemetryStatusTCP(const std::shared_ptr<Client>& client, const TelemetryServerStatusHeader& header) {
    ssize_t totalHeaderBytes = sizeof(header);
    ssize_t bytesLeft = totalHeaderBytes;
    char* headerPtr = reinterpret_cast<char*>(const_cast<TelemetryServerStatusHeader*>(&header));
    while (bytesLeft > 0) {
        ssize_t sent = send(client->getDescriptors().telemetryFd, headerPtr, bytesLeft, 0);
        if (sent == -1) throw std::runtime_error("TelemetryServer::sendTelemetryStatusTCP(): send() failed");
        bytesLeft -= sent;
        headerPtr += sent;
    }
}