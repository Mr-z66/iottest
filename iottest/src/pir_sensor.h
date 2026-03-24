#ifndef PIR_SENSOR_H
#define PIR_SENSOR_H

#include <Arduino.h>
#include "config.h"

#define SENSOR_OK 1
#define SENSOR_ERROR 0
#define SENSOR_DISABLED -1

class PIRSensor {
private:
    int pin;
    bool motionDetected;
    int digitalValue;

    void updateReading();

public:
    explicit PIRSensor(int digitalPin);
    int begin();
    int readData();
    bool isMotionDetected() const;
    bool isIdle() const;
    int getDigitalValue() const;
    void printDebugInfo() const;
};

#endif // PIR_SENSOR_H
