#ifndef RTCLIB_H
#define RTCLIB_H

#include "Arduino.h"

class DateTime {
public:
    DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0) {}
    DateTime(const char* date, const char* time) {}
    DateTime() {}

    uint16_t year() const { return 2023; }
    uint8_t month() const { return 1; }
    uint8_t day() const { return 1; }
    uint8_t hour() const { return 12; }
    uint8_t minute() const { return 0; }
    uint8_t second() const { return 0; }
};

class RTC_DS1307 {
public:
    bool begin() { return true; }
    bool isrunning() { return true; }
    void adjust(const DateTime& dt) {}
    DateTime now() { return DateTime(); }
};

#endif
