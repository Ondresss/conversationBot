#pragma once
#include "SensorConfig.h"
#include "periphery/IMU.h"
#include "TelemetryData.h"
#include "ISensor.h"
#include "TelemetryContext.h"
#include <cstddef>
#include <memory>
#include <stdexcept>
class IMUSensor : public ISensor {
private:
    IMU_ST_ANGLES_DATA stAngles;
    IMU_ST_SENSOR_DATA stGyroRawData;
    IMU_ST_SENSOR_DATA stAccelRawData;
    IMU_ST_SENSOR_DATA stMagnRawData;
    float temperature;
    std::shared_ptr<TelemetryContext> telemetryContext = nullptr;
    std::shared_ptr<IMUSensorConfig> sensorConfig = nullptr;
public:
    explicit IMUSensor(std::shared_ptr<TelemetryContext> telemetryContext,std::shared_ptr<IMUSensorConfig> config) : temperature(0.0f),telemetryContext(telemetryContext),sensorConfig(config) {
        if(!telemetryContext) throw std::runtime_error("IMUSensor: Cannot continue because context wasnt initialized");
        if(!config) throw std::runtime_error("IMUSensor: Cannot continue because config wasnt initialized");
        imuInit();
    }
    void update() override;
    OrientationData calculateOrientationAngles();

    float getTemperature() const {
        return temperature;
    }

    const IMU_ST_SENSOR_DATA& getAccelerometer() const { return stAccelRawData; }
    const IMU_ST_SENSOR_DATA& getGyroscope() const { return stGyroRawData; }
    const IMU_ST_SENSOR_DATA& getMagnetometer() const { return stMagnRawData; }
};