#include "../headers/TelemetryHandler.h"

void TelemetryHandler::startWorker(const std::tuple<TelemetryType, std::shared_ptr<BaseSensorConfig>>& sensor) {
    auto [type, config] = sensor;
    this->sensors[type] = std::make_unique<TelemetryWorker>(sensor, this->telemetryContext);
    this->sensors[type]->start();
    spdlog::info("TelemetryHandler: Started worker for sensor type {}", toStringTelemetryType(type));
}

uint32_t TelemetryHandler::getAllSensorsMask() const {
    uint32_t mask = 0;
    for (const auto& [type, worker] : this->sensors) {
        mask |= (1U << static_cast<uint32_t>(type));
    }
    return mask;
}