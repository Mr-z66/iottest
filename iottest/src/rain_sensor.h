#ifndef RAIN_SENSOR_H
#define RAIN_SENSOR_H

#include <Arduino.h>
#include "config.h"

// ============================================
// MH-RD 雨滴传感器 - 面向对象设计
// ============================================

// 传感器状态码
#define RAIN_SENSOR_OK     1
#define RAIN_SENSOR_ERROR  0

// ============================================
// RainSensor 类定义
// ============================================
class RainSensor {
private:
    int pin;
    bool rainDetected;
    
    void updateReading();

public:
    RainSensor(int digitalPin);
    bool begin();
    bool readData();
    bool isRainDetected() const;
    bool isNoRain() const;
    int getDigitalValue() const;
    void printDebugInfo() const;
};

#endif // RAIN_SENSOR_H