#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <Arduino.h>

class TimeUtils {
public:
    static void begin();
    static String iso8601Now();
    static bool isTimeSynced();
};

#endif // TIME_UTILS_H
