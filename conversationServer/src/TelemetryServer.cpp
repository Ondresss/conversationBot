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
            spdlog::info("Image server: New client connected with IP {}",updatedClient->getIP());
            this->clientThreads.emplace_back(&TelemetryServer::handleClient, this, updatedClient);
        }
    } catch(const std::exception& e) {
        spdlog::error("Telemetry::run() -> TelemetryServer failed: " + std::string(e.what()));
    }
    spdlog::warn("Telemetry Server was stopped");
}


std::vector<TelemetryType> TelemetryServer::recieveAvailableSensorsTCP() {
    TelemetryAvailabilityHeader header;
    ssize_t headerBytesToReceive = sizeof(TelemetryAvailabilityHeader);
    char* headerData = reinterpret_cast<char*>(&header);
    while(headerBytesToReceive > 0) {
        ssize_t receivedBytes = recv(this->fd,
                                    headerData + (sizeof(TelemetryAvailabilityHeader) - headerBytesToReceive),
                                    headerBytesToReceive,
                                    0);
        if(receivedBytes <= 0) {
            throw std::runtime_error("TelemetryClient: receive server status header failed (server disconnected or error)");
        }
        headerBytesToReceive -= receivedBytes;
    }
    spdlog::debug("TelemetryClient: Received server status header with type={}", static_cast<int>(header.type));
    return header;

    return header.decodeSensors(header.noSensors);

}


void TelemetryServer::handleClient(std::shared_ptr<Client> client) {
    auto clientRegistry = this->context->getClientRegistry();
    try {
        std::vector<TelemetryType> availableSensors = this->recieveAvailableSensorsTCP();
        
        while(true) {
            client->getClientSync().acquireWorkerGate();
            ScopeLatchGuard latchGuard(*client);
        }
    } catch(const std::exception& e) {
        spdlog::error("Telemetry::run() -> TelemetryServer failed: " + std::string(e.what()));
    }
    client->disconnect(ServerType::Telemetry);
    clientRegistry->removeClient(client);
}