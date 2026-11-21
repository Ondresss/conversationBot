#pragma once

struct BaseSensorConfig {
    virtual ~BaseSensorConfig() = default;
    float updateRateHz = 100.0f;
};

struct IMUSensorConfig : public BaseSensorConfig {
    float accelRangeG = 2.0f;
    float gyroRangeDPS = 250.0f;
    float magRangeUT = 4800.0f;
};