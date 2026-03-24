#include "core/time_utils.h"

#include <time.h>
#include "core/config.h"

namespace {
time_t g_bootFallbackEpoch = 0;

int monthToNumber(const String& month) {
    if (month == "Jan") return 0;
    if (month == "Feb") return 1;
    if (month == "Mar") return 2;
    if (month == "Apr") return 3;
    if (month == "May") return 4;
    if (month == "Jun") return 5;
    if (month == "Jul") return 6;
    if (month == "Aug") return 7;
    if (month == "Sep") return 8;
    if (month == "Oct") return 9;
    if (month == "Nov") return 10;
    return 11;
}

time_t parseBuildTime() {
    String buildDate = __DATE__;
    String buildTime = __TIME__;

    struct tm timeInfo = {};
    timeInfo.tm_mon = monthToNumber(buildDate.substring(0, 3));
    timeInfo.tm_mday = buildDate.substring(4, 6).toInt();
    timeInfo.tm_year = buildDate.substring(7).toInt() - 1900;
    timeInfo.tm_hour = buildTime.substring(0, 2).toInt();
    timeInfo.tm_min = buildTime.substring(3, 5).toInt();
    timeInfo.tm_sec = buildTime.substring(6, 8).toInt();
    timeInfo.tm_isdst = 0;

    return mktime(&timeInfo);
}
}

void TimeUtils::begin() {
    g_bootFallbackEpoch = parseBuildTime();
    configTime(GMT_OFFSET_SECONDS,
               DAYLIGHT_OFFSET_SECONDS,
               NTP_SERVER_PRIMARY,
               NTP_SERVER_SECONDARY);
}

String TimeUtils::iso8601Now() {
    time_t now = time(nullptr);
    if (now < 100000) {
        now = g_bootFallbackEpoch + static_cast<time_t>(millis() / 1000UL);
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
