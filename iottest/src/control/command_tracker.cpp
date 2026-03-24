#include "control/command_tracker.h"

#include "core/config.h"

CommandTracker::CommandTracker() : entries(), pendingRequests() {
}

CommandDuplicateStatus CommandTracker::checkDuplicate(const CommandRequest& request) const {
    for (const Entry& entry : entries) {
        if (entry.request.requestId == request.requestId) {
            if (entry.request.deviceId == request.deviceId &&
                entry.request.command == request.command &&
                sameParams(entry.request, request)) {
                return COMMAND_DUPLICATE_SAME;
            }
            return COMMAND_DUPLICATE_CONFLICT;
        }
    }
    return COMMAND_NOT_FOUND;
}

bool CommandTracker::getResult(const String& requestId, CommandResult& result) const {
    for (const Entry& entry : entries) {
        if (entry.request.requestId == requestId && entry.status == "FINAL") {
            result = entry.finalResult;
            return true;
        }
    }
    return false;
}

bool CommandTracker::getAccepted(const String& requestId, CommandRequest& request) const {
    for (const Entry& entry : entries) {
        if (entry.request.requestId == requestId) {
            request = entry.request;
            return true;
        }
    }
    return false;
}

void CommandTracker::recordAccepted(const CommandRequest& request) {
    pruneExpired();

    Entry entry;
    entry.request = request;
    entry.status = "ACCEPTED";
    entry.createdAtMs = millis();
    entries.push_back(entry);
    pendingRequests.push_back(request);
}

void CommandTracker::recordFinalResult(const CommandResult& result) {
    pruneExpired();

    for (Entry& entry : entries) {
        if (entry.request.requestId == result.requestId) {
            entry.status = "FINAL";
            entry.finalResult = result;
            return;
        }
    }

    Entry entry;
    entry.request.requestId = result.requestId;
    entry.request.deviceId = result.deviceId;
    entry.status = "FINAL";
    entry.finalResult = result;
    entry.createdAtMs = millis();
    entries.push_back(entry);
}

bool CommandTracker::popNextPending(CommandRequest& request) {
    pruneExpired();
    if (pendingRequests.empty()) {
        return false;
    }

    request = pendingRequests.front();
    pendingRequests.erase(pendingRequests.begin());
    return true;
}

void CommandTracker::pruneExpired() {
    unsigned long now = millis();

    std::vector<Entry> keptEntries;
    keptEntries.reserve(entries.size());
    for (const Entry& entry : entries) {
        if (now - entry.createdAtMs <= COMMAND_IDEMPOTENT_WINDOW_MS) {
            keptEntries.push_back(entry);
        }
    }
    entries = keptEntries;

    std::vector<CommandRequest> keptPending;
    keptPending.reserve(pendingRequests.size());
    for (const CommandRequest& request : pendingRequests) {
        bool exists = false;
        for (const Entry& entry : entries) {
            if (entry.request.requestId == request.requestId && entry.status == "ACCEPTED") {
                exists = true;
                break;
            }
        }
        if (exists) {
            keptPending.push_back(request);
        }
    }
    pendingRequests = keptPending;
}

bool CommandTracker::sameParams(const CommandRequest& left, const CommandRequest& right) const {
    return left.hasDurationSec == right.hasDurationSec &&
           left.durationSec == right.durationSec &&
           left.hasLevel == right.hasLevel &&
           left.level == right.level;
}
