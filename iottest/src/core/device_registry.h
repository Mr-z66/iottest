#ifndef DEVICE_REGISTRY_H
#define DEVICE_REGISTRY_H

#include <Arduino.h>
#include <vector>
#include "core/gateway_models.h"

class DeviceRegistry {
private:
    DeviceModel buildSensorDevice(const String& id,
                                  const String& name,
                                  const String& type,
                                  bool online,
                                  const String& location,
                                  const String& updatedAt,
                                  const String& metricType,
                                  bool metricAvailable,
                                  float value,
                                  const String& unit) const;

public:
    std::vector<DeviceModel> buildDevices(const SensorAvailabilitySnapshot& availability,
                                          const TelemetrySnapshot& telemetry,
                                          const std::vector<ActuatorStatusSnapshot>& actuators) const;
};

#endif // DEVICE_REGISTRY_H
