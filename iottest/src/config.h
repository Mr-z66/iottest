#ifndef CONFIG_H
#define CONFIG_H

// Serial
#define SERIAL_BAUD_RATE 115200

// DHT11
#define DHT_PIN 4
#define DHT_TYPE DHT11

// Shared I2C bus
#define SGP30_SDA 18
#define SGP30_SCL 19

// HW-390 soil moisture
#define SOIL_MOISTURE_PIN 34

// MH-RD rain sensor
#define RAIN_SENSOR_PIN 13

// HC-SR501 PIR sensor
#define PIR_SENSOR_PIN 14

// DS18B20
#define DS18B20_PIN 5
#define DS18B20_ADDRESS 0x28

// I2C addresses
#define SGP30_I2C_ADDRESS 0x58
#define GY302_SDA SGP30_SDA
#define GY302_SCL SGP30_SCL
#define GY302_I2C_ADDRESS 0x23

// System
#define SENSOR_READ_INTERVAL 2000
#define I2C_SCAN_ENABLED false

#endif // CONFIG_H
