#ifndef GY302_SENSOR_H
#define GY302_SENSOR_H

#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>
#include "config.h"

#define SENSOR_OK 1
#define SENSOR_ERROR 0
#define SENSOR_DISABLED -1

class GY302Sensor {
private:
    BH1750 lightMeter;
    float lux;
    bool initialized;
    bool dataValid;

public:
    GY302Sensor();
    int begin();
    int readData();
    float getLux() const;
    bool isDataValid() const;
    void printDebugInfo() const;
};

#endif // GY302_SENSOR_H
