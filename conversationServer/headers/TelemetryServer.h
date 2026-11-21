#pragma once
#include "AbstractServer.h"
#include <stop_token>
#include "TelemetryAvailabilityHeader.h"
#include "TelemetryData.h"
#include "TelemetryServerStatusHeader.h"
#include <vector>
class TelemetryServer : public AbstractServer {
public:
    TelemetryServer(ServerInfo serverInfo, std::shared_ptr<SharedContext> context);
    void run(std::stop_token stopToken) override;
    void handleClient(std::shared_ptr<Client> client) override;
    void disconnectAllClients() override;
private:
    std::vector<TelemetryType> recieveAvailableSensorsTCP();

};