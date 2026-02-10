#include "Arduino.h"
#include <chrono>

unsigned long mock_millis = 0;
int mock_analog_val = 500;

int analogRead(int pin) {
    return mock_analog_val;
}
