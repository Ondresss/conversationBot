#include "../headers/TelemetryWorker.h"
#include <cstdint>
#include <memory>
#include <thread>


void TelemetryWorker::start() {
    this->running = true;
    
    this->workerThread = std::make_unique<std::thread>([this]() {
        this->fetchLoop();
    });
}


void TelemetryWorker::fetchLoop() {
    while (this->running) {
        this->fetchFromSensor();
    }
}


void TelemetryWorker::stop() {
    this->running = false;
    
    if (this->workerThread->joinable()) {
        this->workerThread->join();
    }
}


TelemetryWorker::TelemetryWorker(const std::tuple<TelemetryType, std::shared_ptr<BaseSensorConfig>> s, std::shared_ptr<TelemetryContext> context) {
    auto [type, config] = s;
    switch (type) {
        case TelemetryType::IMU:
            this->sensor = std::make_unique<IMUSensor>(context, std::dynamic_pointer_cast<IMUSensorConfig>(config));
            break;
        default:
            throw std::runtime_error("TelemetryWorker: Unsupported sensor type");
    }
}

void TelemetryWorker::fetchFromSensor() {
    if(this->sensor) {
        this->sensor->update();
    } else {
        throw std::runtime_error("TelemetryWorker: No sensor initialized");
    } 
}