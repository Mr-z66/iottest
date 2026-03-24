#ifndef DS18B20_SENSOR_H
#define DS18B20_SENSOR_H

#include <Arduino.h>
#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// ============================================
// DS18B20 温度传感器类
// ============================================

// 传感器状态码
#define SENSOR_OK     1
#define SENSOR_ERROR  0
#define SENSOR_DISABLED -1

class DS18B20Sensor {
private:
    int pin;                    // 传感器引脚
    float temperature;          // 当前温度值
    bool dataValid;             // 数据有效性标志
    unsigned long lastReadTime; // 上次读取时间
    
    OneWire oneWire;            // OneWire 总线对象
    DallasTemperature sensors;  // DallasTemperature 库对象
    
    // 静态常量
    static const unsigned long READ_INTERVAL = 750;  // 读取间隔（毫秒）
    static constexpr float MIN_TEMP = -20.0f;        // 最小温度（°C）
    static constexpr float MAX_TEMP = 60.0f;         // 最大温度（°C）
    
    // 验证温度读数是否合理
    bool validateReading(float temp);
    
public:
    // 构造函数
    DS18B20Sensor(int sensorPin);
    
    // 初始化传感器
    int begin();
    
    // 读取温度数据
    int readData();
    
    // 获取当前温度
    float getTemperature() const;
    
    // 检查数据是否有效
    bool isDataValid() const;
    
    // 打印调试信息
    void printDebugInfo() const;
    
    // 打印状态信息
    void printStatus() const;
};

#endif // DS18B20_SENSOR_H
