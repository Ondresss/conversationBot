#pragma once
#include <unordered_map>
#include "TelemetryClientData.h"
#include <memory>
#include "Client.h"
class TelemetryServerCache {
public:
    TelemetryServerCache() = default;
    ~TelemetryServerCache() = default;
    void updateTelemetryData(const std::shared_ptr<Client>& client, const std::shared_ptr<TelemetryClientData>& telemetryData) {
        if (!client) {
            throw std::invalid_argument("TelemetryCache::updateTelemetryData() Client pointer is null");
        }
        if (!telemetryData) {
            throw std::invalid_argument("TelemetryCache::updateTelemetryData() Telemetry data pointer is null");
        }
        std::unique_lock lock(mtx); 
        cache[client->getId()] = telemetryData;
    }
    std::shared_ptr<TelemetryClientData> getTelemetryData(const std::shared_ptr<Client>& client) const {
        if (!client) {
            throw std::invalid_argument("TelemetryCache::getTelemetryData() Client pointer is null");
        }
        std::shared_lock lock(mtx); 
        auto it = cache.find(client->getId());
        if (it != cache.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    std::unordered_map<std::size_t, std::shared_ptr<TelemetryClientData>> cache;
    mutable std::shared_mutex mtx;
};