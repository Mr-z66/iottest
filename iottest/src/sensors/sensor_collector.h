#ifndef SENSOR_COLLECTOR_H
#define SENSOR_COLLECTOR_H

#include <Arduino.h>
#include "core/gateway_models.h"
#include "sensors/hw390_sensor.h"
#include "sensors/ds18b20_sensor.h"
#include "sensors/rain_sensor.h"
#include "sensors/pir_sensor.h"
#include "sensors/gy302_sensor.h"

class SensorCollector {
private:
    SensorAvailabilitySnapshot availability;
    HW390Sensor hw390Sensor;
    DS18B20Sensor ds18b20Sensor;
    RainSensor rainSensor;
    PIRSensor pirSensor;
    GY302Sensor gy302Sensor;

public:
    SensorCollector();
    void begin();
    bool sample(TelemetrySnapshot& snapshot);
    SensorAvailabilitySnapshot getAvailability() const;
};

#endif // SENSOR_COLLECTOR_H
