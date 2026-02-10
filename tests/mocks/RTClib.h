#ifndef RTCLIB_H
#define RTCLIB_H

#include "Arduino.h"

class DateTime {
public:
    uint16_t y;
    uint8_t m, d, hh, mm, ss;

    DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0)
    : y(year), m(month), d(day), hh(hour), mm(min), ss(sec) {}

    DateTime(const char* date, const char* time) : y(2023), m(1), d(1), hh(0), mm(0), ss(0) {}
    DateTime() : y(2023), m(1), d(1), hh(0), mm(0), ss(0) {}

    uint16_t year() const { return y; }
    uint8_t month() const { return m; }
    uint8_t day() const { return d; }
    uint8_t hour() const { return hh; }
    uint8_t minute() const { return mm; }
    uint8_t second() const { return ss; }
};

extern DateTime mock_now_val;

class RTC_DS1307 {
public:
    bool begin() { return true; }
    bool isrunning() { return true; }
    void adjust(const DateTime& dt) { mock_now_val = dt; }
    DateTime now() { return mock_now_val; }
};

#endif
