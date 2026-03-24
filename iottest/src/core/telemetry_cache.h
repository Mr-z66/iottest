#ifndef TELEMETRY_CACHE_H
#define TELEMETRY_CACHE_H

#include <Arduino.h>
#include "core/gateway_models.h"

class TelemetryCache {
private:
    TelemetrySnapshot latest;
    bool initialized;

    bool differs(float a, float b) const;

public:
    TelemetryCache();
    bool update(const TelemetrySnapshot& snapshot);
    const TelemetrySnapshot& getLatest() const;
    bool isInitialized() const;
};

#endif // TELEMETRY_CACHE_H
