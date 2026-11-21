#pragma once
#include "TelemetryData.h"
#include <string>
#include <chrono>
#include <sstream>
struct TelemetryClientData {
    std::string timeStamp;
    TelemetryData telemetryData;

    TelemetryClientData(const TelemetryData& data) : telemetryData(data) {
        time_t timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        this->timeStamp = std::string(std::ctime(&timestamp));
    }

    std::string toString() const {
        const auto& ori = this->telemetryData.orientationData;
        std::stringstream ss;
        ss << "Orientation data: [pitch: " << ori.pitch
        << ", roll: " << ori.roll
        << ", yaw: " << ori.yaw << "]";
        return ss.str();
    }

};