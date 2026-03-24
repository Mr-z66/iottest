#ifndef WS_PUBLISHER_H
#define WS_PUBLISHER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <vector>
#include "core/gateway_models.h"

class WsPublisher {
private:
    AsyncWebSocket* webSocket;
    unsigned long lastHeartbeatAt;

    String buildTelemetryPayload(const TelemetrySnapshot& telemetry) const;
    String buildDeviceStatusPayload(const std::vector<DeviceModel>& devices) const;
    String buildCommandResultPayload(const CommandResult& result) const;

public:
    WsPublisher();
    void attach(AsyncWebSocket* socket);
    void begin();
    void loop();
    void publishTelemetry(const TelemetrySnapshot& telemetry);
    void publishDeviceStatus(const std::vector<DeviceModel>& devices);
    void publishCommandResult(const CommandResult& result);
};

#endif // WS_PUBLISHER_H
