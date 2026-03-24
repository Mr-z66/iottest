#ifndef REST_SERVER_H
#define REST_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "core/gateway_models.h"
#include "network/wifi_manager.h"
#include "core/telemetry_cache.h"
#include "sensors/sensor_collector.h"
#include "core/device_registry.h"
#include "control/actuator_controller.h"
#include "control/command_tracker.h"
#include "network/ws_publisher.h"

class RestServer {
private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    WifiManager& wifiManager;
    TelemetryCache& telemetryCache;
    SensorCollector& sensorCollector;
    DeviceRegistry& deviceRegistry;
    ActuatorController& actuatorController;
    CommandTracker& commandTracker;
    WsPublisher& wsPublisher;

    void addCommonHeaders(AsyncWebServerResponse* response);
    void handleGatewayStatus(AsyncWebServerRequest* request);
    void handleTelemetryRealtime(AsyncWebServerRequest* request);
    void handleDevices(AsyncWebServerRequest* request);
    void handleCommandPost(AsyncWebServerRequest* request, const String& deviceId, const String& body);
    void handleCommandQuery(AsyncWebServerRequest* request, const String& requestId);
    GatewayStatusSnapshot buildGatewayStatus() const;
    std::vector<DeviceModel> buildDevices() const;

public:
    RestServer(uint16_t port,
               WifiManager& wifiManagerRef,
               TelemetryCache& telemetryCacheRef,
               SensorCollector& sensorCollectorRef,
               DeviceRegistry& deviceRegistryRef,
               ActuatorController& actuatorControllerRef,
               CommandTracker& commandTrackerRef,
               WsPublisher& wsPublisherRef);

    void begin();
    void loop();
};

#endif // REST_SERVER_H
