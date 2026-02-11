# Technical Manual: Seasonal Autonomous Solar Tracker

**Project:** Mechatronics 400 (Irish Climate Optimized)
**Student:** Richard
**Mentor:** Mechatronics Project 400

This manual consolidates the system architecture, electrical logic, and physical assembly protocols for the Seasonal Solar Tracker.

## 1. Assembly Overview (Quick Steps)

*   **Infrastructure:** Set up the 12V Power Bus and Master Kill Switch.
*   **Logic Core:** Stack the Data Logger Shield onto the Arduino and provide 12V power.
*   **Sensing:** Install LDRs with appropriate resistors/pull-ups.
*   **Motion:** Connect the H-Bridge logic pins to the Arduino and power pins to the Bus Bar.
*   **Lighting:** Connect the LED lighting system to the control pin.
*   **Actuation:** Connect the Linear Actuator to the H-Bridge output.

## 2. Hardware Logic & Pin Reference

The software (`main.cpp`) is configured to interact with the following pins. Accuracy here is critical for the state machine to function.

| Pin | Role | Electronic Logic |
| :--- | :--- | :--- |
| **A0** | LDR East | Analog (0-1023). Higher value = more light. |
| **A1** | LDR West | Analog (0-1023). |
| **D7** | LED Lights | Digital Out. HIGH = On, LOW = Off. |
| **D8** | Retract Cmd | Digital Out. Triggers H-Bridge to move East. |
| **D9** | Extend Cmd | Digital Out. Triggers H-Bridge to move West. |
| **D10** | SD Chip Select | SPI Communication for data logging. |
| **A4/A5** | I2C (RTC) | Timekeeping communication (DS1307). |

## 3. Detailed Wiring Protocol

### Phase 1: The 12V Power Rail
*   **Safety First:** Install the Master Kill Switch on the positive battery lead, followed immediately by a 15A Fuse.
*   **Bus Bars:** All components must share a Common Ground. Connect the negative terminal of the battery to your negative bus bar. Every "GND" wire in the system connects here.

### Phase 2: The Sensor & Lighting Subsystem
*   **LDR Voltage Dividers:** Connect one leg of each LDR to 5V. Connect the other leg to the analog pin (A0/A1) and a 10kΩ resistor going to GND. This converts light resistance into measurable voltage.
*   **LED Control:** Connect the LED driver/relay logic pin to **D7**. Ensure the LEDs are powered appropriately (likely via relay or MOSFET from 12V if high power).

### Phase 3: The H-Bridge & Actuator
The H-Bridge is the high-power interface. It uses three distinct connection "clusters":

*   **Logic Cluster (Header Pins):**
    *   IN1 (or PWM A) → Arduino D9 (Extend).
    *   IN2 (or PWM B) → Arduino D8 (Retract).
    *   GND → Arduino GND (for signal reference).
*   **Power Cluster (Screw Terminals):**
    *   VCC/12V → Positive (+) Bus Bar.
    *   GND → Negative (-) Bus Bar.
*   **Motor Cluster (Screw Terminals):**
    *   OUT1/OUT2 → The two wires of the Linear Actuator.

## 4. Operational Strategy: The Irish Context

*   **Strategic Dormancy:** If the RTC detects months 11, 12, 1, or 2, the system stays in `STATE_STRATEGIC_DORMANCY` to prevent battery depletion during the Irish winter. *(Note: This feature is currently disabled in the firmware for testing purposes).*
*   **Sensor Health:** If readings are < 10 or > 1015, the system switches to **Redundant Mode** (`STATE_REDUNDANT`) which uses time-based dead reckoning to ensure the panels keep moving even if moisture or salt air damages the LDR wiring.
*   **Night Reset & Lighting:**
    *   At **16:00**, if sensors read dark (East & West < 8), the system enters Night Mode.
    *   **Action 1:** The LED Lights turn **ON**.
    *   **Action 2:** The panel fully retracts (East) to the home position.
    *   **Duration:** The LEDs stay on for a maximum of **4 hours** or until **Midnight (00:00)**, whichever comes first.
    *   **Wake Up:** The system waits for morning light (> 150) or 7:00 AM to reset to Idle.
