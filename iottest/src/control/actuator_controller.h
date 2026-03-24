#ifndef ACTUATOR_CONTROLLER_H
#define ACTUATOR_CONTROLLER_H

#include <Arduino.h>
#include <vector>
#include "core/gateway_models.h"

class ActuatorController {
private:
    ActuatorStatusSnapshot pump;
    ActuatorStatusSnapshot growLight;

    void initializeActuator(ActuatorStatusSnapshot& actuator,
                            const String& id,
                            const String& name,
                            const String& type,
                            bool enabled);
    void writePump(bool on);
    void writeGrowLight(bool on);
    void updateTimestamp(ActuatorStatusSnapshot& actuator);
    ActuatorStatusSnapshot* findActuator(const String& deviceId);
    const ActuatorStatusSnapshot* findActuator(const String& deviceId) const;

public:
    ActuatorController();
    void begin();
    void loop();
    std::vector<ActuatorStatusSnapshot> getSnapshots() const;
    bool hasDevice(const String& deviceId) const;
    bool validateCommand(const CommandRequest& request, String& errorMessage) const;
    bool executeCommand(const CommandRequest& request, CommandResult& result);
};

#endif // ACTUATOR_CONTROLLER_H
