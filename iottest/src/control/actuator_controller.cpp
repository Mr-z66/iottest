#include "control/actuator_controller.h"

#include "core/config.h"
#include "core/time_utils.h"

ActuatorController::ActuatorController() : pump(), growLight() {
}

void ActuatorController::initializeActuator(ActuatorStatusSnapshot& actuator,
                                            const String& id,
                                            const String& name,
                                            const String& type,
                                            bool enabled) {
    actuator.id = id;
    actuator.name = name;
    actuator.type = type;
    actuator.enabled = enabled;
    actuator.online = enabled;
    actuator.on = false;
    actuator.level = 0;
    actuator.hasLevel = false;
    actuator.autoOffAtMs = 0;
    actuator.updatedAt = TimeUtils::iso8601Now();
}

void ActuatorController::begin() {
    initializeActuator(pump, "pump-001", "Main Pump", "PUMP", ENABLE_PUMP_ACTUATOR);
    initializeActuator(growLight, "growlight-001", "Grow Light", "LIGHT", ENABLE_GROW_LIGHT_ACTUATOR);

    if (pump.enabled) {
        pinMode(PUMP_RELAY_PIN, OUTPUT);
        writePump(false);
    }

    if (growLight.enabled) {
        pinMode(GROW_LIGHT_RELAY_PIN, OUTPUT);
        writeGrowLight(false);
    }
}

void ActuatorController::loop() {
    unsigned long now = millis();
    if (pump.enabled && pump.on && pump.autoOffAtMs > 0 && now >= pump.autoOffAtMs) {
        writePump(false);
        pump.autoOffAtMs = 0;
        updateTimestamp(pump);
    }
}

std::vector<ActuatorStatusSnapshot> ActuatorController::getSnapshots() const {
    std::vector<ActuatorStatusSnapshot> actuators;
    actuators.push_back(pump);
    actuators.push_back(growLight);
    return actuators;
}

bool ActuatorController::hasDevice(const String& deviceId) const {
    return deviceId == pump.id || deviceId == growLight.id;
}

bool ActuatorController::executeCommand(const CommandRequest& request, CommandResult& result) {
    result.requestId = request.requestId;
    result.deviceId = request.deviceId;
    result.timestamp = TimeUtils::iso8601Now();

    ActuatorStatusSnapshot* actuator = findActuator(request.deviceId);
    if (actuator == nullptr || !actuator->enabled) {
        result.result = "FAILED";
        result.finalStatus = "OFFLINE";
        result.message = "device not found or disabled";
        return false;
    }

    if (request.command == "TURN_ON") {
        if (request.deviceId == pump.id) {
            writePump(true);
            if (request.hasDurationSec) {
                int clampedDuration = request.durationSec;
                if (clampedDuration < 0) clampedDuration = 0;
                if (clampedDuration > PUMP_MAX_DURATION_SEC) clampedDuration = PUMP_MAX_DURATION_SEC;
                pump.autoOffAtMs = millis() + (static_cast<unsigned long>(clampedDuration) * 1000UL);
            } else {
                pump.autoOffAtMs = 0;
            }
            updateTimestamp(pump);
            result.result = "SUCCESS";
            result.finalStatus = "ON";
            result.message = "pump started";
            return true;
        }

        writeGrowLight(true);
        updateTimestamp(growLight);
        result.result = "SUCCESS";
        result.finalStatus = "ON";
        result.message = "grow light turned on";
        return true;
    }

    if (request.command == "TURN_OFF") {
        if (request.deviceId == pump.id) {
            writePump(false);
            pump.autoOffAtMs = 0;
            updateTimestamp(pump);
            result.result = "SUCCESS";
            result.finalStatus = "OFF";
            result.message = "pump stopped";
            return true;
        }

        writeGrowLight(false);
        updateTimestamp(growLight);
        result.result = "SUCCESS";
        result.finalStatus = "OFF";
        result.message = "grow light turned off";
        return true;
    }

    result.result = "FAILED";
    result.finalStatus = actuator->on ? "ON" : "OFF";
    result.message = "unsupported command";
    return false;
}

void ActuatorController::writePump(bool on) {
    digitalWrite(PUMP_RELAY_PIN, on ? PUMP_ACTIVE_LEVEL : !PUMP_ACTIVE_LEVEL);
    pump.on = on;
}

void ActuatorController::writeGrowLight(bool on) {
    digitalWrite(GROW_LIGHT_RELAY_PIN, on ? GROW_LIGHT_ACTIVE_LEVEL : !GROW_LIGHT_ACTIVE_LEVEL);
    growLight.on = on;
}

void ActuatorController::updateTimestamp(ActuatorStatusSnapshot& actuator) {
    actuator.updatedAt = TimeUtils::iso8601Now();
}

ActuatorStatusSnapshot* ActuatorController::findActuator(const String& deviceId) {
    if (deviceId == pump.id) return &pump;
    if (deviceId == growLight.id) return &growLight;
    return nullptr;
}
