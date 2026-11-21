#pragma once
#include "TelemetryType.h"
#include <cstdint>
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
};
#pragma pack(pop)