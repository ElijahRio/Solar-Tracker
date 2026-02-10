#include <iostream>
#include <chrono>
#include <vector>

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

// Override analogRead to simulate real Arduino timing (~100us)
int analogRead(int pin) {
    auto start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() < 100) {
        // busy wait to simulate ADC conversion time
    }
    return 500; // Return a middle value
}

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
    std::cout << "Established Baseline for runIdleState..." << std::endl;

    // Set state to IDLE and ensure we don't return early
    currentState = STATE_IDLE;
    lastTrackTime = 0;
    // rtc.now() will return a fixed time in mock, let's assume it's daytime

    const int ITERATIONS = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; i++) {
        runIdleState();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Total Time for " << ITERATIONS << " iterations: " << elapsed.count() << " seconds" << std::endl;
    std::cout << "Average Time per runIdleState call: " << (elapsed.count() / ITERATIONS) * 1e6 << " us" << std::endl;

    return 0;
}
