#include <Arduino.h>
#include "core/config.h"
#include "core/time_utils.h"
#include "network/wifi_manager.h"
#include "sensors/sensor_collector.h"
#include "core/telemetry_cache.h"
#include "core/device_registry.h"
#include "control/actuator_controller.h"
#include "control/command_tracker.h"
#include "network/rest_server.h"
#include "network/ws_publisher.h"

SensorCollector sensorCollector;
TelemetryCache telemetryCache;
DeviceRegistry deviceRegistry;
ActuatorController actuatorController;
CommandTracker commandTracker;
WifiManager wifiManager(WIFI_SSID, WIFI_PASSWORD);
WsPublisher wsPublisher;
RestServer restServer(GATEWAY_HTTP_PORT,
                      wifiManager,
                      telemetryCache,
                      sensorCollector,
                      deviceRegistry,
                      actuatorController,
                      commandTracker,
                      wsPublisher);

unsigned long lastSampleAt = 0;
unsigned long lastTelemetryPushAt = 0;

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    delay(100);

    Serial.println("==================================");
    Serial.println("ESP32 Device Service");
    Serial.println("==================================");

    TimeUtils::begin();
    sensorCollector.begin();
    actuatorController.begin();
    wifiManager.begin();

    TelemetrySnapshot initialSnapshot;
    sensorCollector.sample(initialSnapshot);
    telemetryCache.update(initialSnapshot);

    wsPublisher.begin();
    restServer.begin();

    Serial.print("HTTP port: ");
    Serial.println(GATEWAY_HTTP_PORT);
    Serial.print("WebSocket path: ");
    Serial.println(GATEWAY_WS_PATH);
}

void loop() {
    wifiManager.loop();
    actuatorController.loop();
    restServer.loop();
    wsPublisher.loop();

    CommandRequest pendingRequest;
    if (commandTracker.popNextPending(pendingRequest)) {
        CommandResult result;
        actuatorController.executeCommand(pendingRequest, result);
        commandTracker.recordFinalResult(result);

        std::vector<DeviceModel> devices = deviceRegistry.buildDevices(sensorCollector.getAvailability(),
                                                                       telemetryCache.getLatest(),
                                                                       actuatorController.getSnapshots());
        wsPublisher.publishCommandResult(result);
        wsPublisher.publishDeviceStatus(devices);
    }

    unsigned long now = millis();
    if (now - lastSampleAt >= SENSOR_READ_INTERVAL) {
        lastSampleAt = now;

        TelemetrySnapshot snapshot;
        sensorCollector.sample(snapshot);
        bool changed = telemetryCache.update(snapshot);

        if (changed || now - lastTelemetryPushAt >= TELEMETRY_PUSH_INTERVAL_MS) {
            std::vector<DeviceModel> devices = deviceRegistry.buildDevices(sensorCollector.getAvailability(),
                                                                           telemetryCache.getLatest(),
                                                                           actuatorController.getSnapshots());
            wsPublisher.publishTelemetry(telemetryCache.getLatest());
            wsPublisher.publishDeviceStatus(devices);
            lastTelemetryPushAt = now;
        }
    }
}
