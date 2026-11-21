#pragma once
#include "IMUSensor.h"
#include "TelemetryContext.h"
#include "TelemetryData.h"
#include "TelemetryType.h"
#include <atomic>
#include <memory>
#include <thread>
#include <tuple>
#include <vector>
#include "IWorker.h"
#include "SensorConfig.h"
#include <unordered_map>
#include <thread>
class TelemetryWorker : public IWorker {
public:
    TelemetryWorker(const std::tuple<TelemetryType, std::shared_ptr<BaseSensorConfig>> sensor, std::shared_ptr<TelemetryContext> context);
    void fetchFromSensor();
    void start() override;
    void stop() override;
    void fetchLoop() override;
private:
    std::unique_ptr<ISensor> sensor = nullptr;
    std::unique_ptr<std::thread> workerThread = nullptr;
};