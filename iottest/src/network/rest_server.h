#ifndef REST_SERVER_H
#define REST_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
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
    WebServer server;
    WifiManager& wifiManager;
    TelemetryCache& telemetryCache;
    SensorCollector& sensorCollector;
    DeviceRegistry& deviceRegistry;
    ActuatorController& actuatorController;
    CommandTracker& commandTracker;
    WsPublisher& wsPublisher;

    void addCommonHeaders();
    void handleGatewayStatus();
    void handleTelemetryRealtime();
    void handleDevices();
    void handleCommandPost(const String& deviceId);
    void handleCommandQuery(const String& requestId);
    void handleNotFound();
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
