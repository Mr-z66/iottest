#include "rain_sensor.h"

// ============================================
// RainSensor 类实现
// ============================================

/**
 * @brief 构造函数
 * @param digitalPin 数字引脚编号
 */
RainSensor::RainSensor(int digitalPin) : pin(digitalPin) {
    rainDetected = false;
}

/**
 * @brief 初始化传感器
 * @return 总是返回 RAIN_SENSOR_OK
 */
bool RainSensor::begin() {
    pinMode(pin, INPUT);
    
    // 初始读取一次
    readData();
    
    Serial.printf("Rain sensor initialized on GPIO%d\n", pin);
    Serial.println("Note: MH-RD sensor - LOW = rain detected, HIGH = no rain");
    
    return RAIN_SENSOR_OK;
}

/**
 * @brief 读取传感器数据
 * @return 总是返回 true（数字传感器不会失败）
 */
bool RainSensor::readData() {
    updateReading();
    return true;
}

/**
 * @brief 获取是否检测到雨滴
 * @return true表示有雨，false表示无雨
 */
bool RainSensor::isRainDetected() const {
    return rainDetected;
}

/**
 * @brief 获取是否无雨（与isRainDetected相反）
 * @return true表示无雨，false表示有雨
 */
bool RainSensor::isNoRain() const {
    return !rainDetected;
}

/**
 * @brief 获取数字引脚原始值
 * @return 数字值（LOW或HIGH）
 */
int RainSensor::getDigitalValue() const {
    return digitalRead(pin);
}

/**
 * @brief 打印调试信息
 */
void RainSensor::printDebugInfo() const {
    int digitalValue = digitalRead(pin);
    Serial.printf("DEBUG: Rain sensor GPIO%d = %d (Rain: %s)\n",
                  pin,
                  digitalValue,
                  rainDetected ? "true" : "false");
}

/**
 * @brief 更新内部读数（私有方法）
 */
void RainSensor::updateReading() {
    int digitalValue = digitalRead(pin);
    rainDetected = (digitalValue == LOW);
}