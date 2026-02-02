# Seasonal Autonomous Solar Tracker

**Mechatronics 400 Student Project**
**Author:** Richard Riordan

## Overview

This project implements a robust, autonomous solar tracking system using an Arduino. It is designed to maximize solar panel efficiency while incorporating advanced features for weather safety, seasonal energy preservation (hibernation), and hardware failure redundancy.

The system uses Light Dependent Resistors (LDRs) to track the sun's position and a linear actuator to adjust the panel's angle. It intelligently manages power by sleeping between moves and entering a dormant state during winter months or low-light conditions.

## Key Features

*   **Interval Tracking:** Moves every 10 minutes to maintain ~95% efficiency while reducing power consumption.
*   **Automatic Winter Hibernation:** Automatically detects winter months (November - February) via RTC and enters a low-power dormant mode.
*   **Wind Safety Override:** Detects high wind conditions and moves the panel to a safe position to prevent damage.
*   **Sensor Redundancy:** Falls back to "Dead Reckoning" (time-based movement) if light sensors fail or provide inconsistent readings.
*   **Strategic Dormancy:** Enters dormancy during low-light days (storms/heavy clouds) to save power.
*   **Data Logging:** Logs all events, sensor readings, and movements to an SD card in CSV format.
*   **Night Reset:** Automatically resets the panel to the East position at night, ready for the next morning.
*   **Optimized Logging:** Uses efficient string handling to minimize memory usage and maximize performance.

## Hardware Requirements

*   **Microcontroller:** Arduino UNO or Mega.
*   **Shield:** Data Logging Shield (containing SD Card slot + RTC).
    *   *Note:* Requires CR1220 coin cell battery for RTC.
*   **Sensors:**
    *   2x LDRs (Light Dependent Resistors) for East/West detection.
    *   Wind Sensor (Switch type).
*   **Actuation:**
    *   Motor Driver.
    *   Linear Actuator.
*   **Power Supply:** Appropriate power for the Arduino and Motor/Actuator.

## Pin Configuration

| Component | Arduino Pin | Description |
| :--- | :--- | :--- |
| **LDR East** | A0 | Analog input for East light sensor |
| **LDR West** | A1 | Analog input for West light sensor |
| **Wind Sensor** | 2 | Digital Input (Input Pullup, Active HIGH) |
| **Actuator Extend** | 9 | Digital Output to Motor Driver |
| **Actuator Retract** | 10 | Digital Output to Motor Driver |
| **SD Card CS** | 10 | Chip Select for SD Card |

> **⚠️ Warning:** The current code defines both `ACT_RETRACT` and `CHIP_SELECT` on Pin 10. This is likely a conflict if using a standard Data Logging Shield which requires Pin 10 for CS. Please verify your wiring and adjust `ACT_RETRACT` to an available digital pin (e.g., Pin 8) if necessary.

## Software Dependencies

Ensure the following libraries are installed in your Arduino IDE:

1.  **RTClib** (by Adafruit) - For Real Time Clock management.
2.  **SD** (Standard Arduino Library) - For SD card logging.
3.  **SPI** (Standard Arduino Library) - For SPI communication.
4.  **Wire** (Standard Arduino Library) - For I2C communication.

## System Logic (State Machine)

The system operates based on a finite state machine:

1.  **STATE_IDLE:** Monitors sensors and time. Waits for the tracking interval or specific conditions (Night, Wind).
2.  **STATE_TRACKING:** Actively reads LDRs and adjusts the actuator to minimize the difference between East and West sensors.
3.  **STATE_NIGHT_RESET:** Moves the panel back to the East position after sunset.
4.  **STATE_WIND_SAFETY:** Triggered by the wind sensor. Moves panel to a safety position until wind subsides.
5.  **STATE_STRATEGIC_DORMANCY:** Used during Winter (Nov-Feb) or extremely dark days. Minimizes movement to save power.
6.  **STATE_REDUNDANT:** Backup mode. If sensors fail, the system moves the panel West by a fixed amount based on time ("Dead Reckoning").
7.  **STATE_ERROR:** Critical failure state (e.g., RTC missing).

## Data Logging

The system logs data to `datalog.csv` on the SD card.
**Format:** `Date,Time,Event,East,West,Diff`

**Example Log:**
```csv
2023/10/15,10:30,TRACKING,800,750,50
2023/10/15,10:40,TRACKING,810,810,0
2023/10/15,18:00,NIGHT_RESET,100,80,20
```

### Serial Data Dump
You can retrieve the logged data without removing the SD card by sending the character `'d'` or `'D'` via the Serial Monitor.

## Setup Instructions

1.  **Install Libraries:** Use the Arduino Library Manager to install `RTClib`.
2.  **Wiring:** Connect components according to the Pin Configuration table (mind the Pin 10 conflict note).
3.  **RTC Setup:** Insert the CR1220 battery. The code will automatically set the RTC to the compile time if it detects the clock has stopped.
4.  **Upload:** Connect the Arduino via USB and upload `main.cpp`.
5.  **Monitor:** Open Serial Monitor at **9600 baud** to view system status and debug messages.

## License

This project is open for educational use.
