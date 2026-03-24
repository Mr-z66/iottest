#ifndef GATEWAY_MODELS_H
#define GATEWAY_MODELS_H

#include <Arduino.h>
#include <vector>

struct WifiSnapshot {
    bool connected = false;
    String ssid;
    String ip;
    int rssi = 0;
};

struct GatewayStatusSnapshot {
    String gatewayId;
    bool online = false;
    String lastHeartbeat;
    String firmwareVersion;
    int connectedDeviceCount = 0;
    WifiSnapshot wifi;
    uint32_t uptimeSec = 0;
};

struct TelemetrySnapshot {
    bool hasTemperature = false;
    float temperature = 0.0f;
    bool hasHumidity = false;
    float humidity = 0.0f;
    bool hasLight = false;
    float light = 0.0f;
    bool hasCo2 = false;
    uint16_t co2 = 0;
    bool hasTvoc = false;
    uint16_t tvoc = 0;
    bool hasSoilMoisture = false;
    float soilMoisture = 0.0f;
    bool hasSoilTemperature = false;
    float soilTemperature = 0.0f;
    bool hasRainDetected = false;
    bool rainDetected = false;
    bool hasMotionDetected = false;
    bool motionDetected = false;
    String timestamp;
};

struct SensorAvailabilitySnapshot {
    bool dht11 = false;
    bool sgp30 = false;
    bool hw390 = false;
    bool ds18b20 = false;
    bool rain = false;
    bool pir = false;
    bool gy302 = false;
};

struct DeviceTelemetryValue {
    bool available = false;
    String metricType;
    float value = 0.0f;
    String unit;
};

struct DeviceModel {
    String id;
    String name;
    String type;
    String category;
    bool online = false;
    String status;
    int level = 0;
    bool hasLevel = false;
    String location;
    std::vector<String> capabilities;
    DeviceTelemetryValue telemetry;
    String updatedAt;
};

struct ActuatorStatusSnapshot {
    String id;
    String name;
    String type;
    bool enabled = false;
    bool online = false;
    bool on = false;
    int level = 0;
    bool hasLevel = false;
    unsigned long autoOffAtMs = 0;
    String updatedAt;
};

struct CommandRequest {
    String deviceId;
    String command;
    String requestId;
    bool hasDurationSec = false;
    int durationSec = 0;
    bool hasLevel = false;
    int level = 0;
};

struct CommandResult {
    String requestId;
    String deviceId;
    String result;
    String finalStatus;
    String message;
    String timestamp;
};

#endif // GATEWAY_MODELS_H
