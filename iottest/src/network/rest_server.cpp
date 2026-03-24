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
      ws(GATEWAY_WS_PATH),
      wifiManager(wifiManagerRef),
      telemetryCache(telemetryCacheRef),
      sensorCollector(sensorCollectorRef),
      deviceRegistry(deviceRegistryRef),
      actuatorController(actuatorControllerRef),
      commandTracker(commandTrackerRef),
      wsPublisher(wsPublisherRef) {
}

void RestServer::begin() {
    ws.onEvent([](AsyncWebSocket* server,
                  AsyncWebSocketClient* client,
                  AwsEventType type,
                  void* arg,
                  uint8_t* data,
                  size_t len) {
        (void)server;
        (void)client;
        (void)type;
        (void)arg;
        (void)data;
        (void)len;
    });
    server.addHandler(&ws);
    wsPublisher.attach(&ws);

    server.on("/gateway/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleGatewayStatus(request);
    });

    server.on("/telemetry/realtime", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleTelemetryRealtime(request);
    });

    server.on("/devices", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleDevices(request);
    });

    server.on("/devices/pump-001/command", HTTP_POST,
        [](AsyncWebServerRequest* request) {
            (void)request;
        },
        nullptr,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            if (index != 0 || len != total) {
                return;
            }

            String body;
            body.reserve(len + 1);
            for (size_t i = 0; i < len; ++i) {
                body += static_cast<char>(data[i]);
            }
            handleCommandPost(request, "pump-001", body);
        });

    server.on("/devices/growlight-001/command", HTTP_POST,
        [](AsyncWebServerRequest* request) {
            (void)request;
        },
        nullptr,
        [this](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            if (index != 0 || len != total) {
                return;
            }

            String body;
            body.reserve(len + 1);
            for (size_t i = 0; i < len; ++i) {
                body += static_cast<char>(data[i]);
            }
            handleCommandPost(request, "growlight-001", body);
        });

    server.onNotFound([this](AsyncWebServerRequest* request) {
        if (request->method() == HTTP_GET && request->url().startsWith("/commands/")) {
            handleCommandQuery(request, request->url().substring(String("/commands/").length()));
            return;
        }

        AsyncWebServerResponse* response = request->beginResponse(404, "application/json",
                                                                 "{\"code\":404,\"message\":\"not found\"}");
        addCommonHeaders(response);
        request->send(response);
    });

    server.begin();
}

void RestServer::loop() {
}

void RestServer::addCommonHeaders(AsyncWebServerResponse* response) {
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");
    response->addHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
}

void RestServer::handleGatewayStatus(AsyncWebServerRequest* request) {
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

    AsyncWebServerResponse* response = request->beginResponse(200, "application/json", payload);
    addCommonHeaders(response);
    request->send(response);
}

void RestServer::handleTelemetryRealtime(AsyncWebServerRequest* request) {
    const TelemetrySnapshot& snapshot = telemetryCache.getLatest();

    JsonDocument doc;
    doc["code"] = 0;
    doc["message"] = "ok";

    JsonObject data = doc["data"].to<JsonObject>();
    if (snapshot.hasTemperature) data["temperature"] = snapshot.temperature;
    if (snapshot.hasHumidity) data["humidity"] = snapshot.humidity;
    if (snapshot.hasLight) data["light"] = snapshot.light;
    if (snapshot.hasCo2) {
        data["co2"] = snapshot.co2;
        data["eco2"] = snapshot.co2;
    }
    if (snapshot.hasTvoc) data["tvoc"] = snapshot.tvoc;
    if (snapshot.hasSoilMoisture) data["soilMoisture"] = snapshot.soilMoisture;
    data["timestamp"] = snapshot.timestamp;

    String payload;
    serializeJson(doc, payload);

    AsyncWebServerResponse* response = request->beginResponse(200, "application/json", payload);
    addCommonHeaders(response);
    request->send(response);
}

void RestServer::handleDevices(AsyncWebServerRequest* request) {
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

    AsyncWebServerResponse* response = request->beginResponse(200, "application/json", payload);
    addCommonHeaders(response);
    request->send(response);
}

void RestServer::handleCommandPost(AsyncWebServerRequest* request, const String& deviceId, const String& body) {
    JsonDocument requestDoc;
    DeserializationError error = deserializeJson(requestDoc, body);
    if (error) {
        AsyncWebServerResponse* response = request->beginResponse(400, "application/json",
                                                                 "{\"code\":400,\"message\":\"invalid json\"}");
        addCommonHeaders(response);
        request->send(response);
        return;
    }

    CommandRequest commandRequest;
    commandRequest.deviceId = deviceId;
    commandRequest.command = requestDoc["command"] | "";
    commandRequest.requestId = requestDoc["requestId"] | "";

    if (requestDoc["params"]["durationSec"].is<int>()) {
        commandRequest.hasDurationSec = true;
        commandRequest.durationSec = requestDoc["params"]["durationSec"].as<int>();
    }

    if (requestDoc["params"]["level"].is<int>()) {
        commandRequest.hasLevel = true;
        commandRequest.level = requestDoc["params"]["level"].as<int>();
    }

    if (commandRequest.command.isEmpty() || commandRequest.requestId.isEmpty()) {
        AsyncWebServerResponse* response = request->beginResponse(
            400, "application/json", "{\"code\":400,\"message\":\"command and requestId are required\"}");
        addCommonHeaders(response);
        request->send(response);
        return;
    }

    CommandDuplicateStatus duplicateStatus = commandTracker.checkDuplicate(commandRequest);
    if (duplicateStatus == COMMAND_DUPLICATE_CONFLICT) {
        AsyncWebServerResponse* response = request->beginResponse(
            409, "application/json", "{\"code\":409,\"message\":\"requestId conflict\"}");
        addCommonHeaders(response);
        request->send(response);
        return;
    }

    if (duplicateStatus == COMMAND_DUPLICATE_SAME) {
        JsonDocument doc;
        doc["code"] = 0;
        doc["message"] = "accepted";
        doc["requestId"] = commandRequest.requestId;
        JsonObject data = doc["data"].to<JsonObject>();
        data["accepted"] = true;
        data["deviceId"] = deviceId;
        data["status"] = "ACCEPTED";

        String payload;
        serializeJson(doc, payload);

        AsyncWebServerResponse* response = request->beginResponse(202, "application/json", payload);
        addCommonHeaders(response);
        request->send(response);
        return;
    }

    String errorMessage;
    if (!actuatorController.validateCommand(commandRequest, errorMessage)) {
        JsonDocument doc;
        doc["code"] = 400;
        doc["message"] = errorMessage;

        String payload;
        serializeJson(doc, payload);

        AsyncWebServerResponse* response = request->beginResponse(400, "application/json", payload);
        addCommonHeaders(response);
        request->send(response);
        return;
    }

    commandTracker.recordAccepted(commandRequest);

    JsonDocument doc;
    doc["code"] = 0;
    doc["message"] = "accepted";
    doc["requestId"] = commandRequest.requestId;
    JsonObject data = doc["data"].to<JsonObject>();
    data["accepted"] = true;
    data["deviceId"] = deviceId;
    data["status"] = "ACCEPTED";

    String payload;
    serializeJson(doc, payload);

    AsyncWebServerResponse* response = request->beginResponse(202, "application/json", payload);
    addCommonHeaders(response);
    request->send(response);
}

void RestServer::handleCommandQuery(AsyncWebServerRequest* request, const String& requestId) {
    CommandResult result;
    if (!commandTracker.getResult(requestId, result)) {
        CommandRequest accepted;
        if (commandTracker.getAccepted(requestId, accepted)) {
            JsonDocument acceptedDoc;
            acceptedDoc["code"] = 0;
            acceptedDoc["message"] = "ok";
            JsonObject data = acceptedDoc["data"].to<JsonObject>();
            data["requestId"] = accepted.requestId;
            data["deviceId"] = accepted.deviceId;
            data["result"] = "PENDING";
            data["finalStatus"] = "ACCEPTED";
            data["message"] = "command accepted and waiting for execution";
            data["timestamp"] = TimeUtils::iso8601Now();

            String payload;
            serializeJson(acceptedDoc, payload);

            AsyncWebServerResponse* response = request->beginResponse(200, "application/json", payload);
            addCommonHeaders(response);
            request->send(response);
            return;
        }

        AsyncWebServerResponse* response = request->beginResponse(
            404, "application/json", "{\"code\":404,\"message\":\"command result not found\"}");
        addCommonHeaders(response);
        request->send(response);
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

    AsyncWebServerResponse* response = request->beginResponse(200, "application/json", payload);
    addCommonHeaders(response);
    request->send(response);
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
