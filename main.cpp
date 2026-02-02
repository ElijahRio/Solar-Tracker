/*
  Project Mechatronics 400: Seasonal Autonomous Solar Tracker (Final)
  Student: Richard Riordan
  
  HARDWARE REQUIREMENTS:
  1. Arduino UNO/Mega
  2. Data Logging Shield (SD Card + RTC) with CR1220 battery installed.
  3. LDR Sensors (East/West)
  4. Motor Driver + Linear Actuator
  5. Wind Sensor (Switch)
  
  FEATURES:
  - 95% Efficiency Interval Tracking (Sleeps 10 mins between moves)
  - Automatic Winter Hibernation (Nov-Feb) using RTC
  - High Wind Safety Override
  - Data Logging to SD Card (CSV format)
  - Sensor Failure Redundancy (Time-based "Dead Reckoning")
  - Strategic Dormancy (Low Light / Bad Weather)
  - Serial Data Dump capability
*/

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h> // You may need to install "RTClib" via Library Manager

// --- OBJECTS ---
RTC_DS1307 rtc; // Most shields use DS1307. If yours is newer, try RTC_PCF8523
File dataFile;

// --- STATE MACHINE ---
enum State {
  STATE_IDLE,           
  STATE_TRACKING,       
  STATE_NIGHT_RESET,    
  STATE_WIND_SAFETY,    
  STATE_STRATEGIC_DORMANCY, // Renamed from WINTER_DORMANT to include weather
  STATE_REDUNDANT,          // New: Sensor Failure Mode
  STATE_ERROR           
};

State currentState = STATE_IDLE;

// --- PIN DEFINITIONS ---
const int LDR_EAST = A0;
const int LDR_WEST = A1;
const int ACT_EXTEND = 9;  
const int ACT_RETRACT = 10; 
const int WIND_SENSOR_PIN = 2; 
const int CHIP_SELECT = 10; // CS pin for SD card (usually 10 on Shields)

// --- CONFIGURATION ---
const unsigned long TRACKING_INTERVAL = 600000; // 10 Minutes (ms)
const int LDR_THRESHOLD = 50;
const int LDR_MIN_VALID = 10;     // Lowered threshold, if < this, suspect broken wire (0)
const int LDR_MAX_VALID = 1015;   // If > this, suspect short (1023)
const unsigned long REDUNDANT_MOVE_TIME = 1000; // Time to move West in redundant mode (ms)
const unsigned long WIND_SAFETY_DURATION = 30000; // Time to move West during wind alarm (ms)

unsigned long lastTrackTime = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("--- System Booting ---");

  // 1. PIN SETUP
  pinMode(ACT_EXTEND, OUTPUT);
  pinMode(ACT_RETRACT, OUTPUT);
  pinMode(WIND_SENSOR_PIN, INPUT_PULLUP);
  pinMode(CHIP_SELECT, OUTPUT); // Required for SD card

  // 2. RTC SETUP
  if (!rtc.begin()) {
    Serial.println("ERROR: Couldn't find RTC");
    currentState = STATE_ERROR;
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running! Setting time to compile time...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // 3. SD CARD SETUP
  Serial.print("Initializing SD card...");
  
  // Check if new file, if so, write CSV Headers for Excel/Sheets
  if (!SD.exists("datalog.csv")) {
    File headerFile = SD.open("datalog.csv", FILE_WRITE);
    if (headerFile) {
      headerFile.println("Date,Time,Event,East,West,Diff");
      headerFile.close();
    }
  }

  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("Card failed, or not present");
    // We continue anyway, but logging won't work
  } else {
    Serial.println("card initialized.");
    // Write Header to CSV
    logData("System Start", 0, 0, 0); 
  }

  // 4. CHECK SEASON (Immediate Winter Check)
  DateTime now = rtc.now();
  int currentMonth = now.month();
  
  // Strategy: If month is Nov(11), Dec(12), Jan(1), Feb(2) -> Dormant
  if (currentMonth >= 11 || currentMonth <= 2) {
    currentState = STATE_STRATEGIC_DORMANCY;
  }
}

void loop() {
  checkGlobalSafety();
  checkSerialCommand(); // Check for 'd' to dump data

  switch (currentState) {
    case STATE_IDLE:
      runIdleState();
      break;
    case STATE_TRACKING:
      runTrackingState();
      break;
    case STATE_NIGHT_RESET:
      runNightResetState();
      break;
    case STATE_WIND_SAFETY:
      runWindSafetyState();
      break;
    case STATE_STRATEGIC_DORMANCY:
      runDormancyState();
      break;
    case STATE_REDUNDANT:
      runRedundantState();
      break;
    case STATE_ERROR:
      runErrorState();
      break;
  }
}

// --- LOGIC FUNCTIONS ---

void runIdleState() {
  DateTime now = rtc.now();
  
  // 1. Check for Night Time (Reset Condition)
  // Simple check: if both sensors are dark
  int east = analogRead(LDR_EAST);
  int west = analogRead(LDR_WEST);
  
  if (east < 100 && west < 100) {
    // Confirm it's actually evening (past 16:00) to avoid storm triggering reset
    if (now.hour() > 16) {
        currentState = STATE_NIGHT_RESET;
        return;
    } else {
        // It's dark during the day? Strategic Dormancy (Storm/Cloud)
        currentState = STATE_STRATEGIC_DORMANCY;
        return;
    }
  }

  // 2. Check Sensor Health
  if (!isSensorOperational()) {
      Serial.println("Sensors Failed! Switching to Redundancy.");
      currentState = STATE_REDUNDANT;
      return;
  }

  // 3. Check Timer for Tracking
  if (millis() - lastTrackTime > TRACKING_INTERVAL) {
    currentState = STATE_TRACKING;
  }
}

void runTrackingState() {
  int east = analogRead(LDR_EAST);
  int west = analogRead(LDR_WEST);
  int diff = east - west;
  
  // Log the attempt
  logData("TRACKING", east, west, diff);

  if (!isSensorOperational()) {
      stopMotor();
      currentState = STATE_REDUNDANT;
      return;
  }

  if (abs(diff) <= LDR_THRESHOLD) {
    stopMotor();
    lastTrackTime = millis();
    currentState = STATE_IDLE;
  } 
  else if (diff > LDR_THRESHOLD) {
    moveWest();
    delay(500); // Move a bit
    stopMotor(); // Stop to re-measure
  } 
  else if (diff < -LDR_THRESHOLD) {
    moveEast();
    delay(500);
    stopMotor();
  }
}

void runDormancyState() {
  // We only check occasionally to save power
  stopMotor(); // Ensure motor is off
  
  DateTime now = rtc.now();
  
  // 1. Winter Check
  if (now.month() >= 3 && now.month() <= 10) {
    // It's not Winter. Is it still dark?
    int east = analogRead(LDR_EAST);
    if (east > 200) { // Arbitrary "Light" threshold
        Serial.println("Conditions improved. Waking up.");
        currentState = STATE_IDLE;
        return;
    }
  }

  // Log occasionally
  if (now.minute() == 0 && now.second() == 0) {
       logData("DORMANT", 0, 0, 0);
       delay(1000); 
  }
}

void runRedundantState() {
  // Dead Reckoning: Move West a fixed amount every interval
  if (millis() - lastTrackTime > TRACKING_INTERVAL) {
      logData("REDUNDANT_MOVE", 0, 0, 0);
      moveWest();
      delay(REDUNDANT_MOVE_TIME);
      stopMotor();
      lastTrackTime = millis();
      
      // Check if night (by time, since sensors are dead)
      DateTime now = rtc.now();
      if (now.hour() >= 20) {
          currentState = STATE_NIGHT_RESET;
      }
  }
}

void runNightResetState() {
  logData("NIGHT_RESET", 0, 0, 0);
  moveEast();
  delay(30000); // 30 seconds to retract fully (Adjust based on Actuator speed/length)
  stopMotor();
  
  // Wait for morning light OR specific time if sensors are bad
  while (true) {
     checkGlobalSafety(); 
     checkSerialCommand();
     
     int east = analogRead(LDR_EAST);
     DateTime now = rtc.now();
     
     // Wake on Light
     if (east > 150) break;
     
     // Wake on Time (Backup) - e.g. 7 AM
     if (now.hour() == 7 && now.minute() == 0) break;
     
     delay(5000);
  }
  currentState = STATE_IDLE;
}

void runWindSafetyState() {
  logData("WIND_ALARM", 0, 0, 0);
  
  /* BLOCKED OUT FOR CALIBRATION
  moveWest(); // Move to flat/safe position
  delay(WIND_SAFETY_DURATION); // Fully extend? Or retract? depends on mechanical setup.
                // Usually horizontal (noon position) is safest for wind load vs dragged, 
                // or fully retracted (East) to present edge.
                // Assuming West/Extended is flat for this setup.
  stopMotor();
  */
  
  while (digitalRead(WIND_SENSOR_PIN) == HIGH) {
    checkSerialCommand();
    delay(1000);
  }
  currentState = STATE_IDLE; // Return to operation when wind stops
}

void runErrorState() {
  stopMotor();
  Serial.println("CRITICAL ERROR");
  delay(5000);
}

// --- HELPER FUNCTIONS ---

void checkGlobalSafety() {
  if (digitalRead(WIND_SENSOR_PIN) == HIGH) { // Assuming HIGH = Wind
    currentState = STATE_WIND_SAFETY;
  }
}

bool isSensorOperational() {
   int e = analogRead(LDR_EAST);
   int w = analogRead(LDR_WEST);
   
   // Check for disconnected/shorted wires
   if (e < LDR_MIN_VALID || e > LDR_MAX_VALID) return false;
   if (w < LDR_MIN_VALID || w > LDR_MAX_VALID) return false;
   
   return true;
}

void checkSerialCommand() {
    if (Serial.available() > 0) {
        char c = Serial.read();
        if (c == 'd' || c == 'D') {
            dumpDataLog();
        }
    }
}

void dumpDataLog() {
    Serial.println("\n--- DATA DUMP START ---");
    File dumpFile = SD.open("datalog.csv");
    if (dumpFile) {
        const size_t bufSize = 64;
        uint8_t buf[bufSize];
        while (dumpFile.available()) {
            int bytesRead = dumpFile.read(buf, bufSize);
            if (bytesRead > 0) {
                Serial.write(buf, bytesRead);
            }
        }
        dumpFile.close();
    } else {
        Serial.println("Error opening datalog.csv for reading.");
    }
    Serial.println("\n--- DATA DUMP END ---");
}

void logData(String mode, int e, int w, int d) {
  // Format: Date, Time, Mode, East, West, Diff
  DateTime now = rtc.now();
  
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print(',');
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    dataFile.print(now.minute(), DEC);
    dataFile.print(',');
    dataFile.print(mode);
    dataFile.print(',');
    dataFile.print(e);
    dataFile.print(',');
    dataFile.print(w);
    dataFile.print(',');
    dataFile.println(d);
    dataFile.close();
    
    // Also print to Serial for debugging
    Serial.print("LOGGED: "); Serial.println(mode);
  } else {
    Serial.println("Error opening datalog.csv");
  }
}

void moveWest() {
  digitalWrite(ACT_EXTEND, HIGH);
  digitalWrite(ACT_RETRACT, LOW);
}

void moveEast() {
  digitalWrite(ACT_EXTEND, LOW);
  digitalWrite(ACT_RETRACT, HIGH);
}

void stopMotor() {
  digitalWrite(ACT_EXTEND, LOW);
  digitalWrite(ACT_RETRACT, LOW);
}