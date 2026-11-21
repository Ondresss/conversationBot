#pragma once
#include "AbstractServer.h"
#include <stop_token>
#include "TelemetryAvailabilityHeader.h"
#include "TelemetryData.h"
#include "TelemetryServerStatusHeader.h"
#include "TelemetryClientDataHeader.h"
#include <vector>
#include "ScopeLatchGuard.h"
class TelemetryServer : public AbstractServer {
public:
    TelemetryServer(ServerInfo serverInfo, std::shared_ptr<SharedContext> context);
    void run(std::stop_token stopToken) override;
    void handleClient(std::shared_ptr<Client> client) override;
    void disconnectAllClients() override;
private:
    std::vector<TelemetryType> recieveAvailableSensorsTCP(const std::shared_ptr<Client>& client);
    void sendTelemetryStatusTCP(const std::shared_ptr<Client>& client, const TelemetryServerStatusHeader& header);
    std::shared_ptr<TelemetryClientData> recieveTelemetryDataTCP(const std::shared_ptr<Client>& client);

};