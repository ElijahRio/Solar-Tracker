#include "Arduino.h"
#include "RTClib.h"

volatile int mock_sink = 0;
SerialClass Serial;

unsigned long mock_millis_val = 0;
int mock_digitalRead_vals[20] = {0};
int mock_analogRead_vals[20] = {0};
int mock_digitalWrite_vals[20] = {0};
int mock_pinMode_vals[20] = {0};

DateTime mock_now_val = DateTime(2023, 6, 1, 12, 0, 0); // Default to Noon June 1st
