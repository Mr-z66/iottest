#ifndef WS_PUBLISHER_H
#define WS_PUBLISHER_H

#include <Arduino.h>
#include <WebSocketsServer.h>
#include <vector>
#include "core/gateway_models.h"

class WsPublisher {
private:
    WebSocketsServer webSocket;

    String buildTelemetryPayload(const TelemetrySnapshot& telemetry) const;
    String buildDeviceStatusPayload(const std::vector<DeviceModel>& devices) const;
    String buildCommandResultPayload(const CommandResult& result) const;

public:
    explicit WsPublisher(uint16_t port);
    void begin();
    void loop();
    void publishTelemetry(const TelemetrySnapshot& telemetry);
    void publishDeviceStatus(const std::vector<DeviceModel>& devices);
    void publishCommandResult(const CommandResult& result);
};

#endif // WS_PUBLISHER_H
