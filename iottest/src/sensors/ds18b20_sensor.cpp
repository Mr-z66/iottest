#include "sensors/ds18b20_sensor.h"

// ============================================
// DS18B20 温度传感器类实现
// ============================================

// 构造函数
DS18B20Sensor::DS18B20Sensor(int sensorPin)
    : pin(sensorPin),
      temperature(0.0f),
      dataValid(false),
      lastReadTime(0),
      oneWire(sensorPin),
      sensors(&oneWire) {
}

// 初始化传感器
int DS18B20Sensor::begin() {
    delay(50); // 等待总线稳定
    
    // 初始化 DallasTemperature 库
    sensors.begin();
    
    // 检查设备数量
    int deviceCount = sensors.getDeviceCount();
    
    Serial.printf("[DS18B20] 初始化 GPIO%d\n", pin);
    Serial.printf("[DS18B20] 发现设备数量: %d\n", deviceCount);
    
    if (deviceCount == 0) {
        Serial.println("[DS18B20] 警告: 未检测到传感器！请检查：");
        Serial.println("  - 接线是否正确 (VCC, GND, DATA)");
        Serial.println("  - DATA 和 VCC 之间是否接 4.7KΩ 上拉电阻");
        Serial.printf("  接线: VCC->3.3V/5V, GND->GND, DATA->GPIO%d\n", pin);
        return SENSOR_ERROR;
    }
    
    // 设置分辨率（12-bit，精度 0.0625°C）
    sensors.setResolution(12);
    Serial.printf("[DS18B20] 分辨率设置为 12-bit (%.4f°C 精度)\n", 0.0625f);
    Serial.printf("[DS18B20] 温度范围: %.1f - %.1f °C\n", MIN_TEMP, MAX_TEMP);
    
    // 读取设备地址
    byte deviceAddress[8];
    if (sensors.getAddress(deviceAddress, 0)) {
        Serial.print("[DS18B20] 设备地址: ");
        for (int i = 0; i < 8; i++) {
            Serial.print(deviceAddress[i], HEX);
            if (i < 7) Serial.print(":");
        }
        Serial.println();
    }
    
    Serial.println("[DS18B20] 传感器就绪");
    
    // 重置状态
    dataValid = false;
    lastReadTime = 0;
    
    return SENSOR_OK;
}

// 读取温度数据
int DS18B20Sensor::readData() {
    unsigned long currentTime = millis();
    
    // 检查是否达到读取间隔（防止读取过快）
    if (currentTime - lastReadTime < READ_INTERVAL) {
        // 未到读取时间，如果数据有效则返回上次数据
        if (dataValid) {
            return SENSOR_OK;
        }
        return SENSOR_ERROR;
    }
    
    // 请求温度转换
    sensors.requestTemperatures();
    
    // 读取第一个设备的温度（摄氏度）
    float tempReading = sensors.getTempCByIndex(0);
    
    // 验证读数
    if (validateReading(tempReading)) {
        temperature = tempReading;
        dataValid = true;
        lastReadTime = currentTime;
        return SENSOR_OK;
    } else {
        dataValid = false;
        return SENSOR_ERROR;
    }
}

// 获取当前温度
float DS18B20Sensor::getTemperature() const {
    return temperature;
}

// 检查数据是否有效
bool DS18B20Sensor::isDataValid() const {
    return dataValid;
}

// 打印调试信息
void DS18B20Sensor::printDebugInfo() const {
    Serial.printf("温度: %.2f °C\n", temperature);
}

// 打印状态信息
void DS18B20Sensor::printStatus() const {
    Serial.println("\nDS18B20 Sensor Status:");
    Serial.printf("连接引脚: GPIO%d\n", pin);
    Serial.printf("数据有效性: %s\n", dataValid ? "有效" : "无效");
    Serial.printf("当前温度: %.2f °C\n", temperature);
    Serial.println("========================================================");
}

// 验证温度读数（私有方法）
bool DS18B20Sensor::validateReading(float temp) {
    // 检查设备是否断开连接（DS18B20 断开时返回 -127）
    if (temp == DEVICE_DISCONNECTED_C) {
        Serial.println("[DS18B20] 错误: 传感器未连接");
        return false;
    }
    
    // 检查温度是否在合理范围内
    if (temp < MIN_TEMP || temp > MAX_TEMP) {
        Serial.printf("[DS18B20] 警告: 异常温度值 %.2f °C\n", temp);
        // 注意：这里只是警告，仍然接受该读数
        // 如果希望严格过滤，可以改为 return false;
    }
    
    return true;
}
