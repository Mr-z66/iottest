#include "control/command_tracker.h"

#include "core/config.h"

CommandTracker::CommandTracker() : entries() {
}

bool CommandTracker::hasRequest(const String& requestId) const {
    for (const Entry& entry : entries) {
        if (entry.requestId == requestId) {
            return true;
        }
    }
    return false;
}

bool CommandTracker::getResult(const String& requestId, CommandResult& result) const {
    for (const Entry& entry : entries) {
        if (entry.requestId == requestId && entry.status == "FINAL") {
            result = entry.finalResult;
            return true;
        }
    }
    return false;
}

void CommandTracker::recordAccepted(const String& requestId, const String& deviceId) {
    pruneExpired();

    for (Entry& entry : entries) {
        if (entry.requestId == requestId) {
            return;
        }
    }

    Entry entry;
    entry.requestId = requestId;
    entry.deviceId = deviceId;
    entry.status = "ACCEPTED";
    entry.createdAtMs = millis();
    entries.push_back(entry);
}

void CommandTracker::recordFinalResult(const CommandResult& result) {
    pruneExpired();

    for (Entry& entry : entries) {
        if (entry.requestId == result.requestId) {
            entry.status = "FINAL";
            entry.finalResult = result;
            return;
        }
    }

    Entry entry;
    entry.requestId = result.requestId;
    entry.deviceId = result.deviceId;
    entry.status = "FINAL";
    entry.finalResult = result;
    entry.createdAtMs = millis();
    entries.push_back(entry);
}

void CommandTracker::pruneExpired() {
    unsigned long now = millis();
    std::vector<Entry> kept;
    kept.reserve(entries.size());

    for (const Entry& entry : entries) {
        if (now - entry.createdAtMs <= COMMAND_IDEMPOTENT_WINDOW_MS) {
            kept.push_back(entry);
        }
    }

    entries = kept;
}
