#pragma once
#include <string>
enum class TelemetryType {
    NONE = 0,
    IMU = 1,
};
inline std::string toStringTelemetryType(TelemetryType type) {
    switch (type) {
        case TelemetryType::NONE: return "NONE";
        case TelemetryType::IMU: return "IMU";
        default: return "UNKNOWN";
    }
}