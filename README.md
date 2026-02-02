# Project Hub: Seasonal Solar Tracker

**Student:** Richard
**Mentor:** Mechatronics Project 400
**Project Goal:** To design and build a data-driven, fully autonomous, off-grid solar power system, strategically optimized for the unique challenges of the Irish climate.

## 1. Core Project Strategy

*   **Operational Window:** Seasonal (March - October). This is a strategic decision based on the "Seasonal Solar Tracker" analysis to maximize performance, reduce cost by 35%, and increase reliability.
*   **System Autonomy Target:** 5+ days. The system is designed to run all loads for over 5 days with zero solar input, making it robust against prolonged cloudy weather.
*   **Location:** Longitude 53.006510, Latitude -6.650966, Ireland. Design choices are optimized for this specific latitude and climate.

## 2. Mechanical System Design

*   **Panel Mounting:** Single-axis tracking (East-to-West) with a manual tilt adjustment mechanism.
    *   *Rationale:* Provides the majority of tracking energy gains (approx. 30%) without the high cost and complexity of a full dual-axis system.
*   **Manual Tilt:** The frame will allow for two tilt angle settings to optimize for different sun heights:
    *   *Spring/Autumn Setting (Mar-Apr, Sep-Oct):* Steeper angle (~50-55°).
    *   *Summer Setting (May-Aug):* Shallower angle (~35-40°).
*   **Rotation Mechanism:** 12V DC Linear Actuator.
    *   *Rationale:* Chosen for its extreme power efficiency (uses ~11% of daily power budget vs. a servo's ~69%), which is the critical factor in achieving the 5-day autonomy goal. It is also strong and self-locking against wind.
    *   *Specification:* 12V DC Linear Actuator, 250mm stroke, 500N force, slow travel speed (5-10mm/s), with built-in limit switches and an IP65 weather rating.

## 3. Electrical System Design

*   **System Voltage:** 12V DC. All components are specified for a 12V nominal system.
*   **Power Generation:** Renogy 50W, 12V Monocrystalline Rigid Solar Panel.
*   **Power Storage:** 25Ah, 12V LiFePO4 (Lithium Iron Phosphate) Battery. Chosen for its long lifespan, safety, and cycle durability. Higher capacity (e.g., 30Ah) is acceptable if 25Ah is unavailable.
*   **Power Management:** MPPT Solar Charge Controller (e.g., Victron 75/15). (Upgraded from PWM for superior low-light efficiency and data monitoring).
*   **Power Distribution:** A central dual-row bus bar / terminal strip to provide a clean and organized 12V distribution hub for all loads.

## 4. Control & Sensing System

*   **Microcontroller:** Arduino UNO R3 (or Mega).
*   **Data & Time:** Data Logging Shield add-on, which includes a Real-Time Clock (RTC) and an SD card slot.
    *   *Library:* `RTClib` (Adafruit), `SD`.
*   **RTC Battery:** Requires a CR1220 3V coin cell battery.
*   **Sun Sensing:** Differential light sensor using two 5mm LDRs.
    *   *Configuration:* The two LDRs are placed side-by-side, separated by a small fin to cast a shadow, creating a highly sensitive direction sensor.
    *   *Circuit:* Each LDR is wired in a voltage divider circuit with a 10kΩ resistor.
*   **Wind Sensing:** Digital input (Switch) on Pin 2.
    *   *Status:* Implemented in software (`STATE_WIND_SAFETY`).

### Pin Configuration

| Component | Arduino Pin | Description |
| :--- | :--- | :--- |
| **LDR East** | A0 | Analog Input |
| **LDR West** | A1 | Analog Input |
| **Wind Sensor** | 2 | Digital Input (Active HIGH) |
| **Actuator Extend** | 9 | Digital Output |
| **Actuator Retract** | 8 | Digital Output |
| **SD Card CS** | 10 | SPI Chip Select |

## 5. Safety & Best Practices

*   **Master Power Control:** A heavy-duty Battery Isolator Switch ("Kill Switch") rated for >20A will be installed on the positive line from the battery.
*   **Circuit Protection:** A 15A automotive blade fuse will be installed in-line between the master switch and the battery's positive terminal.
*   **Enclosures:**
    *   *Electronics:* A single weatherproof IP65-rated enclosure will house the Charge Controller, Arduino, H-Bridge, and Bus Bar.
    *   *Battery:* The battery will be housed in a separate, adjacent, ventilated enclosure for maximum safety.
*   **Wiring Standards (European):**
    *   *High Power (Zone 1):* 2.5 mm² solar cable.
    *   *Load Distribution (Zone 2):* 0.75 mm² equipment wire.
    *   *Signal Wires (Zone 3):* 0.5 mm² (22 AWG) jumper wires.
    *   *Colour Code:* Red (Power+), Black (Ground-), Other Colours (Signals).

## 6. System Logic & Software

*   **Application:** Arduino IDE.
*   **Download Location:** https://www.google.com/search?q=https://www.arduino.cc/software
*   **Programming Language:** C++ (simplified for Arduino).
*   **Dependencies:** `SPI`, `SD`, `Wire`, `RTClib`.

### Core Logic (State Machine)

The Arduino runs a continuous loop managing the following states:

1.  **STATE_IDLE:**
    *   Checks time and sensors.
    *   Triggers `STATE_NIGHT_RESET` if dark and past 16:00.
    *   Triggers `STATE_TRACKING` every 10 minutes (600,000 ms).
2.  **STATE_TRACKING:**
    *   Reads East/West LDRs.
    *   Moves Actuator to minimize light difference.
    *   Sleeps if difference is within threshold.
3.  **STATE_NIGHT_RESET:**
    *   Retracts actuator (Moves East) for 30 seconds.
    *   Waits for morning light (>150 value) or 7:00 AM backup.
4.  **STATE_WIND_SAFETY:**
    *   Triggered if Wind Sensor (Pin 2) is HIGH.
    *   Holds position (or moves to safety) until wind stops.
5.  **STATE_STRATEGIC_DORMANCY:**
    *   **Winter:** Nov-Feb. Sleeps to save power.
    *   **Dark Days:** If light is too low during day, enters dormancy.
6.  **STATE_REDUNDANT:**
    *   Triggered if sensors fail (readings < 10 or > 1015).
    *   Uses "Dead Reckoning" to move West by time (1000ms move / 10 mins).

### Data Logging
*   Logs to `datalog.csv` on SD Card.
*   Format: `Date,Time,Event,East,West,Diff`
*   **Serial Dump:** Send 'd' in Serial Monitor to read log.
