#pragma once
#include "TelemetryWorker.h"
#include "TelemetryType.h"
#include <cstddef>
#include <memory>
#include <unordered_map>
#include "SensorConfig.h"
#include <spdlog/spdlog.h>
#include <cstdint>
#include "SensorMenu.h"
class TelemetryHandler {
public:
    TelemetryHandler(const std::unordered_map<TelemetryType, std::shared_ptr<BaseSensorConfig>>& sensorConfigs) : telemetryContext(std::make_shared<TelemetryContext>()), sensorConfigs(sensorConfigs) {
        this->startWorkers();
    }
    TelemetryHandler();
    void startWorker(const std::tuple<TelemetryType, std::shared_ptr<BaseSensorConfig>>& sensorConfig);
    void startWorkers();
    uint32_t getAllSensorsMask() const;
    std::size_t getNoSensors() const {
        return sensors.size();
    }
private:
    std::shared_ptr<TelemetryContext> telemetryContext = nullptr;
    std::unordered_map<TelemetryType, std::unique_ptr<TelemetryWorker>> sensors;
    std::unordered_map<TelemetryType, std::shared_ptr<BaseSensorConfig>> sensorConfigs;
};