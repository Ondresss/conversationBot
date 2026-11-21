#pragma once
#include "TelemetryType.h"
#include <cstdint>
#include <vector>
#pragma pack(push, 1)
struct TelemetryAvailabilityHeader {
    uint32_t activeSensorsMask = 0; 
    uint32_t noSensors = 0;
    void setSensor(TelemetryType type) {
        activeSensorsMask |= (1U << static_cast<uint32_t>(type));
    }
  
    bool hasSensor(TelemetryType type) const {
        return (activeSensorsMask & (1U << static_cast<uint32_t>(type))) != 0;
    }
    std::vector<TelemetryType> decodeSensors(int noTypes) {
        std::vector<TelemetryType> telemetryTypes;
        for(int i = 1; i <= noTypes; ++i) {
            TelemetryType type = static_cast<TelemetryType>(i);
            if(this->hasSensor(type)) {
                telemetryTypes.push_back(type);
            }
        }
        return telemetryTypes;
    }
}; 
#pragma pack(pop)