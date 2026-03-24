#include <Arduino.h>
#include "config.h"
#include "dht11_sensor.h"
#include "sgp30_sensor.h"
#include "hw390_sensor.h"
#include "ds18b20_sensor.h"
#include "rain_sensor.h"
#include "pir_sensor.h"
#include "gy302_sensor.h"

bool dht11Available = false;
bool sgp30Available = false;
bool hw390Available = false;
bool ds18b20Available = false;
bool rainSensorAvailable = false;
bool pirSensorAvailable = false;
bool gy302Available = false;

HW390Sensor hw390Sensor(SOIL_MOISTURE_PIN);
DS18B20Sensor ds18b20Sensor(DS18B20_PIN);
RainSensor rainSensor(RAIN_SENSOR_PIN);
PIRSensor pirSensor(PIR_SENSOR_PIN);
GY302Sensor gy302Sensor;

void printSensorStatus(int status);
void printSystemStatus();

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(100);

  Serial.println("==================================");
  Serial.println("ESP32 多传感器系统（模块化架构）");
  Serial.println("==================================");
  Serial.println();
  Serial.println("正在初始化传感器...");

  if (initDHT11() == SENSOR_OK) {
    dht11Available = true;
    Serial.println("DHT11 初始化成功");
  } else {
    Serial.println("DHT11 初始化失败");
  }

  if (initSGP30() == SENSOR_OK) {
    sgp30Available = true;
    Serial.println("SGP30 初始化成功");
  } else {
    Serial.println("SGP30 初始化失败");
  }

  if (initHW390() == SENSOR_OK) {
    hw390Available = true;
    Serial.println("HW-390 初始化成功");
  } else {
    Serial.println("HW-390 初始化失败");
  }

  if (ds18b20Sensor.begin() == SENSOR_OK) {
    ds18b20Available = true;
    Serial.println("DS18B20 初始化成功");
  } else {
    Serial.println("DS18B20 初始化失败");
  }

  if (rainSensor.begin()) {
    rainSensorAvailable = true;
    Serial.println("MH-RD 雨滴传感器初始化成功");
  } else {
    Serial.println("MH-RD 雨滴传感器初始化失败");
  }

  if (pirSensor.begin() == SENSOR_OK) {
    pirSensorAvailable = true;
    Serial.println("HC-SR501 红外传感器初始化成功");
  } else {
    Serial.println("HC-SR501 红外传感器初始化失败");
  }

  if (gy302Sensor.begin() == SENSOR_OK) {
    gy302Available = true;
    Serial.println("GY-302 光照传感器初始化成功");
  } else {
    Serial.println("GY-302 光照传感器初始化失败");
  }

  Serial.println();
  Serial.println("==================================");
  Serial.println("系统就绪，开始读取数据...");
  Serial.println("==================================");
  Serial.println();
}

void loop() {
  delay(SENSOR_READ_INTERVAL);

  Serial.println("----------------------------------");
  Serial.print("运行时间: ");
  Serial.print(millis() / 1000);
  Serial.println(" 秒");
  Serial.println();

  float humidity = 0.0f;
  float temperature = 0.0f;
  int dhtStatus = dht11Available ? readDHT11(humidity, temperature) : SENSOR_DISABLED;
  Serial.println("[传感器 DHT11]");
  if (dhtStatus == SENSOR_OK) {
    Serial.print("  湿度: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("  温度: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
  printSensorStatus(dhtStatus);
  Serial.println();

  uint16_t tvoc = 0;
  uint16_t eco2 = 0;
  int sgpStatus = sgp30Available ? readSGP30(tvoc, eco2) : SENSOR_DISABLED;
  Serial.println("[传感器 SGP30]");
  if (sgpStatus == SENSOR_OK) {
    Serial.print("  TVOC: ");
    Serial.print(tvoc);
    Serial.println(" ppb");
    Serial.print("  eCO2: ");
    Serial.print(eco2);
    Serial.println(" ppm");
  }
  printSensorStatus(sgpStatus);
  Serial.println();

  Serial.println("[传感器 HW-390]");
  if (hw390Available && hw390Sensor.readData()) {
    int moisture = hw390Sensor.getPercentage();
    Serial.print("  土壤湿度: ");
    Serial.print(moisture);
    Serial.println(" %");
    Serial.print("  原始值: ");
    Serial.println(hw390Sensor.getRawValue());

    if (moisture < 20) {
      Serial.println("  土壤状态: 干燥，建议浇水");
    } else if (moisture < 50) {
      Serial.println("  土壤状态: 偏干，可适量补水");
    } else if (moisture < 80) {
      Serial.println("  土壤状态: 湿润，状态良好");
    } else {
      Serial.println("  土壤状态: 偏湿，注意排水");
    }
    printSensorStatus(SENSOR_OK);
  } else {
    printSensorStatus(hw390Available ? SENSOR_ERROR : SENSOR_DISABLED);
  }
  Serial.println();

  int ds18b20Status = ds18b20Available ? ds18b20Sensor.readData() : SENSOR_DISABLED;
  Serial.println("[传感器 DS18B20]");
  if (ds18b20Status == SENSOR_OK && ds18b20Sensor.isDataValid()) {
    Serial.print("  温度: ");
    Serial.print(ds18b20Sensor.getTemperature());
    Serial.println(" °C");
  }
  printSensorStatus(ds18b20Status);
  Serial.println();

  Serial.println("[传感器 MH-RD 雨滴]");
  if (rainSensorAvailable && rainSensor.readData()) {
    Serial.print("  数字电平: ");
    Serial.println(rainSensor.getDigitalValue());
    if (rainSensor.isRainDetected()) {
      Serial.println("  状态说明: 检测到雨滴");
    } else {
      Serial.println("  状态说明: 当前无雨");
    }
    printSensorStatus(SENSOR_OK);
  } else {
    printSensorStatus(rainSensorAvailable ? SENSOR_ERROR : SENSOR_DISABLED);
  }
  Serial.println();

  int pirStatus = pirSensorAvailable ? pirSensor.readData() : SENSOR_DISABLED;
  Serial.println("[传感器 HC-SR501]");
  if (pirStatus == SENSOR_OK) {
    Serial.print("  数字电平: ");
    Serial.println(pirSensor.getDigitalValue());
    if (pirSensor.isMotionDetected()) {
      Serial.println("  状态说明: 检测到人体移动");
    } else {
      Serial.println("  状态说明: 当前无人移动");
    }
  }
  printSensorStatus(pirStatus);
  Serial.println();

  int gy302Status = gy302Available ? gy302Sensor.readData() : SENSOR_DISABLED;
  Serial.println("[传感器 GY-302]");
  if (gy302Status == SENSOR_OK && gy302Sensor.isDataValid()) {
    float lux = gy302Sensor.getLux();
    Serial.print("  光照强度: ");
    Serial.print(lux, 2);
    Serial.println(" lux");

    if (lux < 10.0f) {
      Serial.println("  环境判断: 极暗");
    } else if (lux < 100.0f) {
      Serial.println("  环境判断: 较暗");
    } else if (lux < 1000.0f) {
      Serial.println("  环境判断: 室内正常光照");
    } else {
      Serial.println("  环境判断: 强光环境");
    }
  }
  printSensorStatus(gy302Status);
  Serial.println();

  printSystemStatus();
  Serial.println();
}

void printSensorStatus(int status) {
  Serial.print("  读取状态: ");
  switch (status) {
    case SENSOR_OK:
      Serial.println("正常");
      break;
    case SENSOR_ERROR:
      Serial.println("失败");
      break;
    case SENSOR_DISABLED:
      Serial.println("未启用");
      break;
    default:
      Serial.println("未知");
      break;
  }
}

void printSystemStatus() {
  Serial.println("========== 本次读取状态汇总 ==========");
  Serial.print("DHT11: ");
  Serial.println(dht11Available ? "已启用" : "未启用");
  Serial.print("SGP30: ");
  Serial.println(sgp30Available ? "已启用" : "未启用");
  Serial.print("HW-390: ");
  Serial.println(hw390Available ? "已启用" : "未启用");
  Serial.print("DS18B20: ");
  Serial.println(ds18b20Available ? "已启用" : "未启用");
  Serial.print("MH-RD: ");
  Serial.println(rainSensorAvailable ? "已启用" : "未启用");
  Serial.print("HC-SR501: ");
  Serial.println(pirSensorAvailable ? "已启用" : "未启用");
  Serial.print("GY-302: ");
  Serial.println(gy302Available ? "已启用" : "未启用");
  Serial.println("==================================");
}
