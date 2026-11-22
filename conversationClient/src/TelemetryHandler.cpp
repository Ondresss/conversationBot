#include "../headers/TelemetryHandler.h"

void TelemetryHandler::startWorker(const std::tuple<TelemetryType, std::shared_ptr<BaseSensorConfig>>& sensor) {
    auto [type, config] = sensor;
    this->sensors[type] = std::make_unique<TelemetryWorker>(sensor, this->telemetryContext);
    this->sensors[type]->start();
    spdlog::info("TelemetryHandler: Started worker for sensor type {}", toStringTelemetryType(type));
}

void TelemetryHandler::startWorkers() {
    for (const auto& [type, config] : this->sensorConfigs) {
        this->startWorker(std::make_tuple(type, config));
    }
}

TelemetryHandler::TelemetryHandler() : telemetryContext(std::make_shared<TelemetryContext>()) {
    SensorMenu sensorMenu;
    sensorMenu.draw();
    auto selectedSensors = sensorMenu.getSelectedSensors();
    for (const auto& sensorInfo : selectedSensors) {
        this->sensorConfigs[sensorInfo.type] = sensorInfo.config;
    }
    this->startWorkers();
}


uint32_t TelemetryHandler::getAllSensorsMask() const {
    uint32_t mask = 0;
    for (const auto& [type, worker] : this->sensors) {
        mask |= (1U << static_cast<uint32_t>(type));
    }
    return mask;
}