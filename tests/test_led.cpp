#include <iostream>
#include <vector>
#include <cassert>

#include "Arduino.h"
#include "RTClib.h"
#include "SD.h"
#include "SPI.h"
#include "Wire.h"

// Define global mock objects required by main.cpp
SDClass SD;

// Forward declarations for functions in main.cpp
void checkSerialCommand();
void dumpDataLog();
void logData(const char* mode, int e, int w, int d);
void runIdleState();
void runTrackingState();
void runNightResetState();
void runDormancyState();
void runRedundantState();
void runErrorState();
bool isSensorOperational();
void moveWest();
void moveEast();
void stopMotor();

// Include application code
#include "../main.cpp"

// Helper to reset state
void reset_test_env() {
    currentState = STATE_IDLE;
    lastTrackTime = 0;
    mock_millis_val = 0;
    // Reset pins
    for(int i=0; i<20; i++) {
        mock_digitalRead_vals[i] = LOW;
        mock_digitalWrite_vals[i] = LOW;
        mock_analogRead_vals[i] = 0;
    }
    // Set default safe sensor values
    mock_analogRead_vals[LDR_EAST] = 500;
    mock_analogRead_vals[LDR_WEST] = 500;

    // Set default time to Noon
    mock_now_val = DateTime(2023, 6, 1, 12, 0, 0);
}

void test_evening_trigger() {
    std::cout << "Test: Evening Trigger..." << std::endl;
    reset_test_env();

    // 1. Set time to 18:00
    mock_now_val = DateTime(2023, 6, 1, 18, 0, 0);

    // 2. Set LDRs to dark (< 8)
    mock_analogRead_vals[LDR_EAST] = 4;
    mock_analogRead_vals[LDR_WEST] = 4;

    // 3. Run loop to trigger transition
    loop();

    // 4. Check State
    if (currentState != STATE_NIGHT_RESET) {
        std::cout << "FAIL: State should be STATE_NIGHT_RESET (2), got " << currentState << std::endl;
        exit(1);
    }

    // 5. Run loop again to execute Night Reset logic (LED ON)
    loop();

    // 6. Check LED Pin (D7) is HIGH
    if (mock_digitalWrite_vals[7] != HIGH) {
        std::cout << "FAIL: LED Pin 7 should be HIGH" << std::endl;
        exit(1);
    }

    // 7. Check Retract (D8) is HIGH
    if (mock_digitalWrite_vals[8] != HIGH) {
        std::cout << "FAIL: Retract Pin 8 should be HIGH" << std::endl;
        exit(1);
    }

    std::cout << "PASS" << std::endl;
}

void test_led_duration() {
    std::cout << "Test: LED Duration (4 hours)..." << std::endl;
    reset_test_env();

    // Trigger evening
    mock_now_val = DateTime(2023, 6, 1, 18, 0, 0);
    mock_analogRead_vals[LDR_EAST] = 4;
    mock_analogRead_vals[LDR_WEST] = 4;
    loop(); // To Idle
    loop(); // To Night Reset -> LED ON

    if (mock_digitalWrite_vals[7] != HIGH) {
         std::cout << "SETUP FAIL: LED did not turn on" << std::endl;
         exit(1);
    }

    // Advance time by 4 hours + 1 min
    mock_millis_val += (4 * 3600 * 1000) + 60000;
    mock_now_val = DateTime(2023, 6, 1, 22, 1, 0);

    loop(); // Should detect timeout and turn off LED

    if (mock_digitalWrite_vals[7] != LOW) {
        std::cout << "FAIL: LED Pin 7 should be LOW after 4 hours" << std::endl;
        exit(1);
    }
    std::cout << "PASS" << std::endl;
}

void test_led_midnight() {
    std::cout << "Test: LED Midnight Cutoff..." << std::endl;
    reset_test_env();

    // Trigger evening at 23:00
    mock_now_val = DateTime(2023, 6, 1, 23, 0, 0);
    mock_analogRead_vals[LDR_EAST] = 4;
    mock_analogRead_vals[LDR_WEST] = 4;
    loop();
    loop();

    if (mock_digitalWrite_vals[7] != HIGH) {
        std::cout << "SETUP FAIL: LED failed to turn on at 23:00" << std::endl;
        exit(1);
    }

    // Advance to midnight (1 hour later)
    mock_millis_val += (1 * 3600 * 1000);
    mock_now_val = DateTime(2023, 6, 2, 0, 0, 0); // Midnight next day

    loop();

    if (mock_digitalWrite_vals[7] != LOW) {
        std::cout << "FAIL: LED Pin 7 should be LOW at midnight" << std::endl;
        exit(1);
    }
    std::cout << "PASS" << std::endl;
}

int main() {
    std::cout << "Running LED Logic Tests..." << std::endl;
    setup();

    test_evening_trigger();
    test_led_duration();
    test_led_midnight();

    std::cout << "All Tests Passed!" << std::endl;
    return 0;
}
