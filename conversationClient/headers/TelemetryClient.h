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
    explicit TelemetryClient(ServerInfo info,std::shared_ptr<TelemetryHandler> handler) : AbstractClient(info), telemetryHandler(handler) {
        if(!this->telemetryHandler) throw std::runtime_error("TelemetryClient: telemetryHandler is null");
    }
    void run() override;
private:
    void sendAvailableSensorsTCP() const;
    void sendTelemetryAvailabilityHeaderTCP(const TelemetryAvailabilityHeader& header) const;
    TelemetryServerStatusHeader recieveServerStatusHeaderTCP() const;
    void sendTelemetryDataTCP(const TelemetryData& data) const;
    void runFetchMode();

    std::unique_ptr<TelemetryHandler> telemetryHandler = nullptr;
};