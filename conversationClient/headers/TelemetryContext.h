#pragma once
#include "TelemetryData.h"
#include <mutex>
class TelemetryContext {
public:
    void updateOrientation(const OrientationData& orientationData);
    TelemetryData fetchData();
private:
    TelemetryData data;
    std::mutex dataMutex;
};