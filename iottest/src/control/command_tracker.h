#ifndef COMMAND_TRACKER_H
#define COMMAND_TRACKER_H

#include <Arduino.h>
#include <vector>
#include "core/gateway_models.h"

enum CommandDuplicateStatus {
    COMMAND_NOT_FOUND = 0,
    COMMAND_DUPLICATE_SAME = 1,
    COMMAND_DUPLICATE_CONFLICT = 2
};

class CommandTracker {
private:
    struct Entry {
        CommandRequest request;
        String status;
        CommandResult finalResult;
        unsigned long createdAtMs;
    };

    std::vector<Entry> entries;
    std::vector<CommandRequest> pendingRequests;

    void pruneExpired();
    bool sameParams(const CommandRequest& left, const CommandRequest& right) const;

public:
    CommandTracker();
    CommandDuplicateStatus checkDuplicate(const CommandRequest& request) const;
    bool getResult(const String& requestId, CommandResult& result) const;
    bool getAccepted(const String& requestId, CommandRequest& request) const;
    void recordAccepted(const CommandRequest& request);
    void recordFinalResult(const CommandResult& result);
    bool popNextPending(CommandRequest& request);
};

#endif // COMMAND_TRACKER_H
