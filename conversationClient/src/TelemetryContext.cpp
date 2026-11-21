#include "../headers/TelemetryContext.h"
#include <mutex>

void TelemetryContext::updateOrientation(const OrientationData& orientationData) {
    std::lock_guard<std::mutex> lock(this->dataMutex);

    this->data.orientationData = orientationData;
}

TelemetryData TelemetryContext::fetchData() {
    std::lock_guard<std::mutex> lock(this->dataMutex);
    return this->data;
}