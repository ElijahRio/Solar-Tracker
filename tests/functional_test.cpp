#include <iostream>
#include <cassert>

// Include mocks
#include "mocks/Arduino.h"
#include "mocks/SD.h"
#include "mocks/RTClib.h"
#include "mocks/SPI.h"
#include "mocks/Wire.h"

// Define global mock objects
SerialClass Serial;
SDClass SD;
volatile int mock_sink = 0;

// Forward declarations
void checkGlobalSafety();
void checkSerialCommand();
void runIdleState();
void runTrackingState();
void runNightResetState();
void runWindSafetyState();
void runDormancyState();
void runRedundantState();
void runErrorState();
bool isSensorOperational(int e, int w);
void stopMotor();
void moveWest();
void moveEast();
void dumpDataLog();
void logData(const char* mode, int e, int w, int d);

// Include the application code
#include "../main.cpp"

int main() {
    std::cout << "Running Functional Tests..." << std::endl;

    // Test 1: isSensorOperational
    assert(isSensorOperational(500, 500) == true);
    assert(isSensorOperational(5, 500) == false); // Too low
    assert(isSensorOperational(500, 1020) == false); // Too high
    std::cout << "Test 1 Passed: isSensorOperational works as expected." << std::endl;

    // Test 2: runIdleState transition to Tracking
    currentState = STATE_IDLE;
    lastTrackTime = 0;
    mock_millis = TRACKING_INTERVAL + 1;
    mock_analog_val = 500; // Bright enough
    runIdleState();
    assert(currentState == STATE_TRACKING);
    std::cout << "Test 2 Passed: runIdleState transitions to STATE_TRACKING when bright." << std::endl;

    // Test 3: runIdleState transition to Dormancy (dark during day)
    currentState = STATE_IDLE;
    mock_analog_val = 50; // Dark
    // Mock RTC to return 12:00 (noon)
    // We need to make sure rtc.now().hour() <= 16
    // Looking at RTClib.h mock...
    runIdleState();
    assert(currentState == STATE_STRATEGIC_DORMANCY);
    std::cout << "Test 3 Passed: runIdleState transitions to STATE_STRATEGIC_DORMANCY when dark during day." << std::endl;

    // Test 4: runTrackingState behavior
    currentState = STATE_TRACKING;
    mock_analog_val = 500; // Balanced
    mock_millis = 0;
    lastTrackTime = 0;
    runTrackingState();
    assert(currentState == STATE_IDLE);
    std::cout << "Test 4 Passed: runTrackingState transitions to STATE_IDLE when balanced." << std::endl;

    std::cout << "All Functional Tests Passed!" << std::endl;

    return 0;
}
