#ifndef COMMAND_TRACKER_H
#define COMMAND_TRACKER_H

#include <Arduino.h>
#include <vector>
#include "core/gateway_models.h"

class CommandTracker {
private:
    struct Entry {
        String requestId;
        String deviceId;
        String status;
        CommandResult finalResult;
        unsigned long createdAtMs;
    };

    std::vector<Entry> entries;

    void pruneExpired();

public:
    CommandTracker();
    bool hasRequest(const String& requestId) const;
    bool getResult(const String& requestId, CommandResult& result) const;
    void recordAccepted(const String& requestId, const String& deviceId);
    void recordFinalResult(const CommandResult& result);
};

#endif // COMMAND_TRACKER_H
