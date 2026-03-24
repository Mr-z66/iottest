#include "network/rest_server.h"

#include <ArduinoJson.h>
#include "core/config.h"
#include "core/time_utils.h"

RestServer::RestServer(uint16_t port,
                       WifiManager& wifiManagerRef,
                       TelemetryCache& telemetryCacheRef,
                       SensorCollector& sensorCollectorRef,
                       DeviceRegistry& deviceRegistryRef,
                       ActuatorController& actuatorControllerRef,
                       CommandTracker& commandTrackerRef,
                       WsPublisher& wsPublisherRef)
    : server(port),
      wifiManager(wifiManagerRef),
      telemetryCache(telemetryCacheRef),
      sensorCollector(sensorCollectorRef),
      deviceRegistry(deviceRegistryRef),
      actuatorController(actuatorControllerRef),
      commandTracker(commandTrackerRef),
      wsPublisher(wsPublisherRef) {
}

void RestServer::begin() {
    server.on("/gateway/status", HTTP_GET, [this]() { handleGatewayStatus(); });
    server.on("/telemetry/realtime", HTTP_GET, [this]() { handleTelemetryRealtime(); });
    server.on("/devices", HTTP_GET, [this]() { handleDevices(); });
    server.onNotFound([this]() { handleNotFound(); });
    server.begin();
}

void RestServer::loop() {
    server.handleClient();
}

void RestServer::addCommonHeaders() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
}

void RestServer::handleGatewayStatus() {
    GatewayStatusSnapshot snapshot = buildGatewayStatus();

    JsonDocument doc;
    doc["code"] = 0;
    doc["message"] = "ok";

    JsonObject data = doc["data"].to<JsonObject>();
    data["gatewayId"] = snapshot.gatewayId;
    data["online"] = snapshot.online;
    data["lastHeartbeat"] = snapshot.lastHeartbeat;
    data["firmwareVersion"] = snapshot.firmwareVersion;
    data["connectedDeviceCount"] = snapshot.connectedDeviceCount;

    JsonObject wifi = data["wifi"].to<JsonObject>();
    wifi["ssid"] = snapshot.wifi.ssid;
    wifi["ip"] = snapshot.wifi.ip;
    wifi["rssi"] = snapshot.wifi.rssi;

    data["uptimeSec"] = snapshot.uptimeSec;

    String payload;
    serializeJson(doc, payload);
    addCommonHeaders();
    server.send(200, "application/json", payload);
}

void RestServer::handleTelemetryRealtime() {
    const TelemetrySnapshot& snapshot = telemetryCache.getLatest();

    JsonDocument doc;
    doc["code"] = 0;
    doc["message"] = "ok";

    JsonObject data = doc["data"].to<JsonObject>();
    if (snapshot.hasTemperature) data["temperature"] = snapshot.temperature;
    if (snapshot.hasHumidity) data["humidity"] = snapshot.humidity;
    if (snapshot.hasLight) data["light"] = snapshot.light;
    if (snapshot.hasCo2) data["co2"] = snapshot.co2;
    if (snapshot.hasSoilMoisture) data["soilMoisture"] = snapshot.soilMoisture;
    data["timestamp"] = snapshot.timestamp;

    String payload;
    serializeJson(doc, payload);
    addCommonHeaders();
    server.send(200, "application/json", payload);
}

void RestServer::handleDevices() {
    std::vector<DeviceModel> devices = buildDevices();

    JsonDocument doc;
    doc["code"] = 0;
    doc["message"] = "ok";
    JsonArray data = doc["data"].to<JsonArray>();

    for (const DeviceModel& device : devices) {
        JsonObject item = data.add<JsonObject>();
        item["id"] = device.id;
        item["name"] = device.name;
        item["type"] = device.type;
        item["category"] = device.category;
        item["online"] = device.online;
        item["status"] = device.status;
        if (device.hasLevel) item["level"] = device.level;
        item["location"] = device.location;

        JsonArray capabilities = item["capabilities"].to<JsonArray>();
        for (const String& capability : device.capabilities) {
            capabilities.add(capability);
        }

        if (device.telemetry.available) {
            JsonObject telemetry = item["telemetry"].to<JsonObject>();
            telemetry["metricType"] = device.telemetry.metricType;
            telemetry["value"] = device.telemetry.value;
            telemetry["unit"] = device.telemetry.unit;
        }

        item["updatedAt"] = device.updatedAt;
    }

    String payload;
    serializeJson(doc, payload);
    addCommonHeaders();
    server.send(200, "application/json", payload);
}

void RestServer::handleCommandPost(const String& deviceId) {
    JsonDocument requestDoc;
    DeserializationError error = deserializeJson(requestDoc, server.arg("plain"));
    if (error) {
        addCommonHeaders();
        server.send(400, "application/json", "{\"code\":400,\"message\":\"invalid json\"}");
        return;
    }

    CommandRequest request;
    request.deviceId = deviceId;
    request.command = requestDoc["command"] | "";
    request.requestId = requestDoc["requestId"] | "";

    if (requestDoc["params"]["durationSec"].is<int>()) {
        request.hasDurationSec = true;
        request.durationSec = requestDoc["params"]["durationSec"].as<int>();
    }

    if (requestDoc["params"]["level"].is<int>()) {
        request.hasLevel = true;
        request.level = requestDoc["params"]["level"].as<int>();
    }

    if (request.command.isEmpty() || request.requestId.isEmpty()) {
        addCommonHeaders();
        server.send(400, "application/json", "{\"code\":400,\"message\":\"command and requestId are required\"}");
        return;
    }

    if (commandTracker.hasRequest(request.requestId)) {
        JsonDocument doc;
        doc["code"] = 0;
        doc["message"] = "accepted";
        doc["requestId"] = request.requestId;
        JsonObject data = doc["data"].to<JsonObject>();
        data["accepted"] = true;
        data["deviceId"] = deviceId;
        data["status"] = "ACCEPTED";

        String payload;
        serializeJson(doc, payload);
        addCommonHeaders();
        server.send(202, "application/json", payload);
        return;
    }

    commandTracker.recordAccepted(request.requestId, request.deviceId);

    CommandResult result;
    actuatorController.executeCommand(request, result);
    commandTracker.recordFinalResult(result);
    wsPublisher.publishCommandResult(result);
    wsPublisher.publishDeviceStatus(buildDevices());

    JsonDocument doc;
    doc["code"] = 0;
    doc["message"] = "accepted";
    doc["requestId"] = request.requestId;
    JsonObject data = doc["data"].to<JsonObject>();
    data["accepted"] = true;
    data["deviceId"] = deviceId;
    data["status"] = "ACCEPTED";

    String payload;
    serializeJson(doc, payload);
    addCommonHeaders();
    server.send(202, "application/json", payload);
}

void RestServer::handleCommandQuery(const String& requestId) {
    CommandResult result;
    if (!commandTracker.getResult(requestId, result)) {
        addCommonHeaders();
        server.send(404, "application/json", "{\"code\":404,\"message\":\"command result not found\"}");
        return;
    }

    JsonDocument doc;
    doc["code"] = 0;
    doc["message"] = "ok";

    JsonObject data = doc["data"].to<JsonObject>();
    data["requestId"] = result.requestId;
    data["deviceId"] = result.deviceId;
    data["result"] = result.result;
    data["finalStatus"] = result.finalStatus;
    data["message"] = result.message;
    data["timestamp"] = result.timestamp;

    String payload;
    serializeJson(doc, payload);
    addCommonHeaders();
    server.send(200, "application/json", payload);
}

void RestServer::handleNotFound() {
    if (server.method() == HTTP_OPTIONS) {
        addCommonHeaders();
        server.send(204, "text/plain", "");
        return;
    }

    const String uri = server.uri();

    if (server.method() == HTTP_POST &&
        uri.startsWith("/devices/") &&
        uri.endsWith("/command")) {
        String deviceId = uri.substring(String("/devices/").length(),
                                        uri.length() - String("/command").length());
        handleCommandPost(deviceId);
        return;
    }

    if (server.method() == HTTP_GET && uri.startsWith("/commands/")) {
        String requestId = uri.substring(String("/commands/").length());
        handleCommandQuery(requestId);
        return;
    }

    addCommonHeaders();
    server.send(404, "application/json", "{\"code\":404,\"message\":\"not found\"}");
}

GatewayStatusSnapshot RestServer::buildGatewayStatus() const {
    GatewayStatusSnapshot snapshot;
    snapshot.gatewayId = GATEWAY_ID;
    snapshot.online = wifiManager.isConnected();
    snapshot.lastHeartbeat = TimeUtils::iso8601Now();
    snapshot.firmwareVersion = FIRMWARE_VERSION;
    snapshot.connectedDeviceCount = static_cast<int>(buildDevices().size());
    snapshot.wifi = wifiManager.getSnapshot();
    snapshot.uptimeSec = millis() / 1000UL;
    return snapshot;
}

std::vector<DeviceModel> RestServer::buildDevices() const {
    return deviceRegistry.buildDevices(sensorCollector.getAvailability(),
                                       telemetryCache.getLatest(),
                                       actuatorController.getSnapshots());
}
