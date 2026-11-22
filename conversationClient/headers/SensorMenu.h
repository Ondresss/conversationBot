#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include <ncurses.h>
#undef OK
#include <spdlog/spdlog.h>

#include "SensorConfig.h"

enum class SensorType {
    None = 0,
    IMU
};

struct SelectedSensorInfo {
    SensorType type{SensorType::None};
    std::string name{"None"};
    bool isConfigured{false};
    std::shared_ptr<BaseSensorConfig> config{nullptr};
};

class SensorMenu {
public:
    SensorMenu() = default;
    ~SensorMenu() = default;

    void draw();

    [[nodiscard]] const std::vector<SelectedSensorInfo>& getSelectedSensors() const {
        return selectedSensors;
    }

    [[nodiscard]] std::shared_ptr<BaseSensorConfig> getConfig(SensorType type) const {
        for (const auto& sensor : selectedSensors) {
            if (sensor.type == type) {
                return sensor.config;
            }
        }
        return nullptr;
    }

private:
    std::vector<SelectedSensorInfo> selectedSensors{};

    bool hasSensor(SensorType type) const;
    void removeSensor(SensorType type);
    SelectedSensorInfo* findSensor(SensorType type);
    void configure(std::shared_ptr<BaseSensorConfig> cfg);
    void configureImu(std::shared_ptr<IMUSensorConfig> cfg);

    int runSelectionMenu(const std::string& title,
                         const std::vector<std::string>& options,
                         int defaultIndex = 0);

    bool editUnsignedIntParam(const std::string& title,
                              const std::string& prompt,
                              unsigned int currentValue,
                              unsigned int& outValue);
};