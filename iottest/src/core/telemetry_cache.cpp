#include "core/telemetry_cache.h"

#include <math.h>

TelemetryCache::TelemetryCache() : latest(), initialized(false) {
}

bool TelemetryCache::differs(float a, float b) const {
    return fabs(a - b) > 0.05f;
}

bool TelemetryCache::update(const TelemetrySnapshot& snapshot) {
    if (!initialized) {
        latest = snapshot;
        initialized = true;
        return true;
    }

    bool changed =
        latest.hasTemperature != snapshot.hasTemperature ||
        latest.hasHumidity != snapshot.hasHumidity ||
        latest.hasLight != snapshot.hasLight ||
        latest.hasCo2 != snapshot.hasCo2 ||
        latest.hasTvoc != snapshot.hasTvoc ||
        latest.hasSoilMoisture != snapshot.hasSoilMoisture ||
        latest.hasSoilTemperature != snapshot.hasSoilTemperature ||
        latest.hasRainDetected != snapshot.hasRainDetected ||
        latest.hasMotionDetected != snapshot.hasMotionDetected ||
        differs(latest.temperature, snapshot.temperature) ||
        differs(latest.humidity, snapshot.humidity) ||
        differs(latest.light, snapshot.light) ||
        latest.co2 != snapshot.co2 ||
        latest.tvoc != snapshot.tvoc ||
        differs(latest.soilMoisture, snapshot.soilMoisture) ||
        differs(latest.soilTemperature, snapshot.soilTemperature) ||
        latest.rainDetected != snapshot.rainDetected ||
        latest.motionDetected != snapshot.motionDetected;

    latest = snapshot;
    return changed;
}

const TelemetrySnapshot& TelemetryCache::getLatest() const {
    return latest;
}

bool TelemetryCache::isInitialized() const {
    return initialized;
}
