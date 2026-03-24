#include "core/time_utils.h"

#include <time.h>
#include "core/config.h"

void TimeUtils::begin() {
    configTime(GMT_OFFSET_SECONDS,
               DAYLIGHT_OFFSET_SECONDS,
               NTP_SERVER_PRIMARY,
               NTP_SERVER_SECONDARY);
}

String TimeUtils::iso8601Now() {
    time_t now = time(nullptr);
    if (now < 100000) {
        return "1970-01-01T00:00:00Z";
    }

    struct tm timeInfo;
    gmtime_r(&now, &timeInfo);

    char buffer[25];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &timeInfo);
    return String(buffer);
}

bool TimeUtils::isTimeSynced() {
    return time(nullptr) >= 100000;
}
