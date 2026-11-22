#include "../headers/TelemetryClient.h"



void TelemetryClient::sendTelemetryDataTCP(const TelemetryData& data) const {
    TelemetryClientDataHeader header;
    header.dataSize = sizeof(TelemetryData);
    const char* headerData = reinterpret_cast<const char*>(&header);
    ssize_t headerBytesToSend = sizeof(TelemetryClientDataHeader);
    while(headerBytesToSend > 0) {
        ssize_t sentBytes = send(this->fd,
                                headerData + (sizeof(TelemetryClientDataHeader) - headerBytesToSend),
                                headerBytesToSend,
                                0);
        if(sentBytes <= 0) {
            throw std::runtime_error("TelemetryClient: send telemetry data header failed (server disconnected or error)");
        }
        headerBytesToSend -= sentBytes;
    }
    spdlog::debug("TelemetryClient: Sent telemetry data header successfully with dataSize={}", header.dataSize);

    const char* dataPtr = reinterpret_cast<const char*>(&data);
    ssize_t dataBytesToSend = sizeof(TelemetryData);
    while(dataBytesToSend > 0) {
        ssize_t sentBytes = send(this->fd,
                                dataPtr + (sizeof(TelemetryData) - dataBytesToSend),
                                dataBytesToSend,
                                0);
        if(sentBytes <= 0) {
            throw std::runtime_error("TelemetryClient: send telemetry data failed (server disconnected or error)");
        }
        dataBytesToSend -= sentBytes;
    }
    spdlog::debug("TelemetryClient: Sent telemetry data successfully");
}

void TelemetryClient::runFetchMode() {
    while(true) {
        auto serverStatusHeader = this->recieveServerStatusHeaderTCP();
        if(serverStatusHeader.status == TelemetryServerStatusType::ERROR) {
            throw std::runtime_error("TelemetryClient: Server responded with ERROR status header during FETCH mode");
        }
        if(serverStatusHeader.type != TelemetryServerControlType::FETCH) {
            throw std::runtime_error("TelemetryClient: Server responded with unexpected control type during FETCH mode");
        }
        TelemetryData data = this->telemetryHandler->fetchData();
        this->sendTelemetryDataTCP(data);
    }
}


void TelemetryClient::run() {
    try {
        if(!this->telemetryHandler) throw std::runtime_error("TelemetryClient: telemetryHandler is null");
        this->connectToServer();
        spdlog::info("Telemetry client running...");
        this->sendAvailableSensorsTCP();
        TelemetryServerStatusHeader statusHeader = this->recieveServerStatusHeaderTCP();
        if(statusHeader.status == TelemetryServerStatusType::ERROR) {
            throw std::runtime_error("TelemetryClient: Server responded with ERROR status header");
        }
        if(statusHeader.type == TelemetryServerControlType::FETCH) {
            this->runFetchMode();
        } else if(statusHeader.type == TelemetryServerControlType::STREAM) {
            throw std::runtime_error("TelemetryClient: STREAM mode not implemented yet");
        } else {
            throw std::runtime_error("TelemetryClient: Unknown server control type received");
        }
    } catch (const std::exception& e) {
        spdlog::error("TelemetryClient: Exception occurred: {}", e.what());
    }
    this->disconnectFromServer();
}


TelemetryServerStatusHeader TelemetryClient::recieveServerStatusHeaderTCP() const {
    TelemetryServerStatusHeader header;
    ssize_t headerBytesToReceive = sizeof(TelemetryServerStatusHeader);
    char* headerData = reinterpret_cast<char*>(&header);
    while(headerBytesToReceive > 0) {
        ssize_t receivedBytes = recv(this->fd,
                                    headerData + (sizeof(TelemetryServerStatusHeader) - headerBytesToReceive),
                                    headerBytesToReceive,
                                    0);
        if(receivedBytes <= 0) {
            throw std::runtime_error("TelemetryClient: receive server status header failed (server disconnected or error)");
        }
        headerBytesToReceive -= receivedBytes;
    }
    spdlog::debug("TelemetryClient: Received server status header with type={}", static_cast<int>(header.type));
    return header;
}

void TelemetryClient::sendAvailableSensorsTCP() const {
    TelemetryAvailabilityHeader header;
    uint32_t mask = this->telemetryHandler->getAllSensorsMask();
    header.activeSensorsMask = mask;
    header.noSensors = this->telemetryHandler->getNoSensors();
    this->sendTelemetryAvailabilityHeaderTCP(header);
}

void TelemetryClient::sendTelemetryAvailabilityHeaderTCP(const TelemetryAvailabilityHeader& header) const {
    spdlog::debug("TelemetryClient: Sending telemetry availability header with activeSensorsMask={} and noSensors={}", header.activeSensorsMask, header.noSensors);
    const char* headerData = reinterpret_cast<const char*>(&header);
    ssize_t headerBytesToSend = sizeof(TelemetryAvailabilityHeader);
    while(headerBytesToSend > 0) {
        ssize_t sentBytes = send(this->fd,
                                headerData + (sizeof(TelemetryAvailabilityHeader) - headerBytesToSend),
                                headerBytesToSend,
                                0);
        if(sentBytes <= 0) {
            throw std::runtime_error("TelemetryClient: send telemetry availability header failed (server disconnected or error)");
        }
        headerBytesToSend -= sentBytes;
    }
    spdlog::debug("TelemetryClient: Sent telemetry availability header successfully");
}