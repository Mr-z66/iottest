#include "core/device_registry.h"

DeviceModel DeviceRegistry::buildSensorDevice(const String& id,
                                              const String& name,
                                              const String& type,
                                              bool online,
                                              const String& location,
                                              const String& updatedAt,
                                              const String& metricType,
                                              bool metricAvailable,
                                              float value,
                                              const String& unit) const {
    DeviceModel device;
    device.id = id;
    device.name = name;
    device.type = type;
    device.category = "SENSOR";
    device.online = online;
    device.status = online ? "READY" : "OFFLINE";
    device.location = location;
    device.updatedAt = updatedAt;
    device.telemetry.available = metricAvailable;
    device.telemetry.metricType = metricType;
    device.telemetry.value = value;
    device.telemetry.unit = unit;
    return device;
}

std::vector<DeviceModel> DeviceRegistry::buildDevices(const SensorAvailabilitySnapshot& availability,
                                                      const TelemetrySnapshot& telemetry,
                                                      const std::vector<ActuatorStatusSnapshot>& actuators) const {
    std::vector<DeviceModel> devices;
    const String updatedAt = telemetry.timestamp;
    const String location = "Zone-A";

    devices.push_back(buildSensorDevice("temp-001", "Ambient Temperature Sensor", "TEMP_SENSOR",
                                        availability.dht11, location, updatedAt,
                                        "TEMPERATURE", telemetry.hasTemperature, telemetry.temperature, "C"));
    devices.push_back(buildSensorDevice("hum-001", "Ambient Humidity Sensor", "HUMIDITY_SENSOR",
                                        availability.dht11, location, updatedAt,
                                        "HUMIDITY", telemetry.hasHumidity, telemetry.humidity, "%"));
    devices.push_back(buildSensorDevice("light-001", "Light Sensor", "LIGHT_SENSOR",
                                        availability.gy302, location, updatedAt,
                                        "LIGHT", telemetry.hasLight, telemetry.light, "lux"));
    devices.push_back(buildSensorDevice("co2-001", "Air Quality Sensor", "CO2_SENSOR",
                                        availability.sgp30, location, updatedAt,
                                        "CO2", telemetry.hasCo2, telemetry.co2, "ppm"));
    devices.push_back(buildSensorDevice("soil-001", "Soil Moisture Sensor", "SOIL_SENSOR",
                                        availability.hw390, location, updatedAt,
                                        "SOIL_MOISTURE", telemetry.hasSoilMoisture, telemetry.soilMoisture, "%"));
    devices.push_back(buildSensorDevice("soil-temp-001", "Soil Temperature Sensor", "SOIL_TEMP_SENSOR",
                                        availability.ds18b20, location, updatedAt,
                                        "SOIL_TEMPERATURE", telemetry.hasSoilTemperature, telemetry.soilTemperature, "C"));
    devices.push_back(buildSensorDevice("rain-001", "Rain Sensor", "RAIN_SENSOR",
                                        availability.rain, location, updatedAt,
                                        "RAIN_DETECTED", telemetry.hasRainDetected, telemetry.rainDetected ? 1.0f : 0.0f, "BOOL"));
    devices.push_back(buildSensorDevice("motion-001", "PIR Motion Sensor", "PIR_SENSOR",
                                        availability.pir, location, updatedAt,
                                        "MOTION_DETECTED", telemetry.hasMotionDetected, telemetry.motionDetected ? 1.0f : 0.0f, "BOOL"));

    for (const ActuatorStatusSnapshot& actuator : actuators) {
        DeviceModel device;
        device.id = actuator.id;
        device.name = actuator.name;
        device.type = actuator.type;
        device.category = "ACTUATOR";
        device.online = actuator.online;
        device.status = actuator.on ? "ON" : "OFF";
        device.hasLevel = actuator.hasLevel;
        device.level = actuator.level;
        device.location = location;
        device.updatedAt = actuator.updatedAt;
        device.capabilities.push_back("TURN_ON");
        device.capabilities.push_back("TURN_OFF");
        devices.push_back(device);
    }

    return devices;
}
