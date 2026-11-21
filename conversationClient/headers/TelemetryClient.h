#pragma once
#include "AbstractClient.h"
#include "TelemetryHandler.h"
#include <memory>
#include <unordered_map>
#include "TelemetryAvailabilityHeader.h"
#include "TelemetryServerStatusHeader.h"
#include "TelemetryClientDataHeader.h"
class TelemetryClient : public AbstractClient {
public:
    explicit TelemetryClient(ServerInfo info,const std::unordered_map<TelemetryType, std::shared_ptr<BaseSensorConfig>>& sensorConfigs) : AbstractClient(info), 
    telemetryHandler(std::make_unique<TelemetryHandler>()) {
        this->startHandlerWorkers(sensorConfigs);
    }
    void run() override;
private:
    void startHandlerWorkers(const std::unordered_map<TelemetryType, std::shared_ptr<BaseSensorConfig>>& sensorConfigs);
    void sendAvailableSensorsTCP() const;
    void sendTelemetryAvailabilityHeaderTCP(const TelemetryAvailabilityHeader& header) const;
    TelemetryServerStatusHeader recieveServerStatusHeaderTCP() const;
    void sendTelemetryDataTCP(const TelemetryData& data) const;
    void runFetchMode();

    std::unique_ptr<TelemetryHandler> telemetryHandler = nullptr;
};