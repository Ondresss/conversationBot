#include "../headers/IMUSensor.h"

OrientationData IMUSensor::calculateOrientationAngles() {
    imuDataGet(&stAngles, &stGyroRawData, &stAccelRawData, &stMagnRawData);
    
    this->temperature = QMI8658_readTemp();
    return OrientationData{
        stAngles.fRoll,
        stAngles.fPitch,
        stAngles.fYaw
    };
}

void IMUSensor::update() {
    auto data = this->calculateOrientationAngles();
    this->telemetryContext->updateOrientation(data);
    float sleepTime = 1.0f / this->sensorConfig->updateRateHz;
    lguSleep(sleepTime);
}
