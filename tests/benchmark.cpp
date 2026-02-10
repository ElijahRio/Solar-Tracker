#include <iostream>
#include <chrono>
#include <vector>

// Include mocks
#include "Arduino.h"
#include "SD.h"
#include "RTClib.h"
#include "SPI.h"
#include "Wire.h"

// Define global mock objects
SDClass SD;

// Forward declarations for functions in main.cpp (required because they are not declared in main.cpp before use, relying on Arduino IDE)
void checkSerialCommand();
void runIdleState();
void runTrackingState();
void runNightResetState();
void runDormancyState();
void runRedundantState();
void runErrorState();
bool isSensorOperational();
void stopMotor();
void moveWest();
void moveEast();
void dumpDataLog();
void logData(const char* mode, int e, int w, int d);

// Include the application code
// We define a macro to prevent duplicate main if we were linking, but here we include cpp.
#include "../../main.cpp"

int main() {
    std::cout << "Starting Benchmark..." << std::endl;

    // Initialize mock environment
    // rtc and dataFile are global in main.cpp, so they are instantiated.

    // Benchmark logData
    const int ITERATIONS = 1000000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; i++) {
        logData("TRACKING", 500, 400, 100);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Time: " << elapsed.count() << " seconds" << std::endl;
    std::cout << "Average: " << (elapsed.count() / ITERATIONS) * 1e6 << " us/call" << std::endl;

    return 0;
}
