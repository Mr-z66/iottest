#include "network/ws_publisher.h"

#include <ArduinoJson.h>

WsPublisher::WsPublisher(uint16_t port) : webSocket(port) {
}

void WsPublisher::begin() {
    webSocket.begin();
}

void WsPublisher::loop() {
    webSocket.loop();
}

void WsPublisher::publishTelemetry(const TelemetrySnapshot& telemetry) {
    webSocket.broadcastTXT(buildTelemetryPayload(telemetry));
}

void WsPublisher::publishDeviceStatus(const std::vector<DeviceModel>& devices) {
    webSocket.broadcastTXT(buildDeviceStatusPayload(devices));
}

void WsPublisher::publishCommandResult(const CommandResult& result) {
    webSocket.broadcastTXT(buildCommandResultPayload(result));
}

String WsPublisher::buildTelemetryPayload(const TelemetrySnapshot& telemetry) const {
    JsonDocument doc;
    doc["topic"] = "telemetry.update";
    doc["messageId"] = String("msg-") + millis();
    doc["timestamp"] = telemetry.timestamp;

    JsonObject data = doc["data"].to<JsonObject>();
    if (telemetry.hasTemperature) data["temperature"] = telemetry.temperature;
    if (telemetry.hasHumidity) data["humidity"] = telemetry.humidity;
    if (telemetry.hasLight) data["light"] = telemetry.light;
    if (telemetry.hasCo2) data["co2"] = telemetry.co2;
    if (telemetry.hasSoilMoisture) data["soilMoisture"] = telemetry.soilMoisture;
    if (telemetry.hasRainDetected) data["rainDetected"] = telemetry.rainDetected;
    if (telemetry.hasMotionDetected) data["motionDetected"] = telemetry.motionDetected;

    String payload;
    serializeJson(doc, payload);
    return payload;
}

String WsPublisher::buildDeviceStatusPayload(const std::vector<DeviceModel>& devices) const {
    JsonDocument doc;
    doc["topic"] = "device.status.changed";
    doc["messageId"] = String("msg-") + millis();
    doc["timestamp"] = devices.empty() ? String("1970-01-01T00:00:00Z") : devices.front().updatedAt;

    JsonArray data = doc["data"].to<JsonArray>();
    for (const DeviceModel& device : devices) {
        JsonObject item = data.add<JsonObject>();
        item["id"] = device.id;
        item["status"] = device.status;
        item["online"] = device.online;
        if (device.hasLevel) {
            item["level"] = device.level;
        }
        item["updatedAt"] = device.updatedAt;
    }

    String payload;
    serializeJson(doc, payload);
    return payload;
}

String WsPublisher::buildCommandResultPayload(const CommandResult& result) const {
    JsonDocument doc;
    doc["topic"] = "command.result";
    doc["messageId"] = String("msg-") + millis();
    doc["timestamp"] = result.timestamp;

    JsonObject data = doc["data"].to<JsonObject>();
    data["requestId"] = result.requestId;
    data["deviceId"] = result.deviceId;
    data["result"] = result.result;
    data["finalStatus"] = result.finalStatus;
    data["message"] = result.message;

    String payload;
    serializeJson(doc, payload);
    return payload;
}
