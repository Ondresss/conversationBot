#pragma once
#include <cstdint>

struct OrientationData {
    float roll;
    float pitch;
    float yaw;
};

struct TelemetryData {
    OrientationData orientationData;
};