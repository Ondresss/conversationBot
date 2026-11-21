#pragma once
#include "TelemetryWorker.h"
#include "TelemetryType.h"
#include <cstddef>
#include <memory>
#include <unordered_map>
#include "SensorConfig.h"
#include <spdlog/spdlog.h>
#include <cstdint>
class TelemetryHandler {
public:
    TelemetryHandler() : telemetryContext(std::make_shared<TelemetryContext>()) {}
    void startWorker(const std::tuple<TelemetryType, std::shared_ptr<BaseSensorConfig>>& sensorConfig);
    uint32_t getAllSensorsMask() const;
    std::size_t getNoSensors() const {
        return sensors.size();
    }
private:
    std::shared_ptr<TelemetryContext> telemetryContext = nullptr;
    std::unordered_map<TelemetryType, std::unique_ptr<TelemetryWorker>> sensors;
};