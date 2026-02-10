# Technical Manual: Seasonal Autonomous Solar Tracker

**Project:** Mechatronics 400 (Irish Climate Optimized)
**Student:** Richard
**Mentor:** Mechatronics Project 400

This manual consolidates the system architecture, electrical logic, and physical assembly protocols for the Seasonal Solar Tracker.

## 1. Assembly Overview (Quick Steps)

*   **Infrastructure:** Set up the 12V Power Bus and Master Kill Switch.
*   **Logic Core:** Stack the Data Logger Shield onto the Arduino and provide 12V power.
*   **Sensing:** Install LDRs and Wind Sensor with appropriate resistors/pull-ups.
*   **Motion:** Connect the H-Bridge logic pins to the Arduino and power pins to the Bus Bar.
*   **Actuation:** Connect the Linear Actuator to the H-Bridge output.

## 2. Hardware Logic & Pin Reference

The software (`main.cpp`) is configured to interact with the following pins. Accuracy here is critical for the state machine to function.

| Pin | Role | Electronic Logic |
| :--- | :--- | :--- |
| **A0** | LDR East | Analog (0-1023). Higher value = more light. |
| **A1** | LDR West | Analog (0-1023). |
| **D2** | Wind Sensor | Digital. INPUT_PULLUP (HIGH by default, LOW on trigger). |
| **D8** | Retract Cmd | Digital Out. Triggers H-Bridge to move East. |
| **D9** | Extend Cmd | Digital Out. Triggers H-Bridge to move West. |
| **D10** | SD Chip Select | SPI Communication for data logging. |
| **A4/A5** | I2C (RTC) | Timekeeping communication (DS1307). |

## 3. Detailed Wiring Protocol

### Phase 1: The 12V Power Rail
*   **Safety First:** Install the Master Kill Switch on the positive battery lead, followed immediately by a 15A Fuse.
*   **Bus Bars:** All components must share a Common Ground. Connect the negative terminal of the battery to your negative bus bar. Every "GND" wire in the system connects here.

### Phase 2: The Sensor Subsystem
*   **LDR Voltage Dividers:** Connect one leg of each LDR to 5V. Connect the other leg to the analog pin (A0/A1) and a 10kΩ resistor going to GND. This converts light resistance into measurable voltage.
*   **Wind Safety:** Connect the wind switch between Pin 2 and GND. The internal pull-up handles the logic; the switch simply closes the circuit to trigger `STATE_WIND_SAFETY`.

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

*   **Strategic Dormancy:** If the RTC detects months 11, 12, 1, or 2, the system stays in `STATE_STRATEGIC_DORMANCY` to prevent battery depletion during the Irish winter.
*   **Sensor Health:** If readings are < 10 or > 1015, the system switches to Redundant Mode (Dead Reckoning) to ensure the panels keep moving even if moisture or salt air damages the LDR wiring.
*   **Night Reset:** At 16:00, if sensors are dark, the system retracts fully (East) to prepare for the next sunrise.
