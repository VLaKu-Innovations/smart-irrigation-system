/**
 * ============================================================================
 * Project: ESP32 2-Zone Smart Drip Irrigation System
 * File: 03_dual_zone_standalone.ino
 * Description: All-in-one standalone automated 2-zone irrigation controller.
 *              Includes non-blocking FSM, dual-sensor averaging per zone,
 *              anti-water-hammer sequencing, and safety watchdogs.
 * Board: DOIT ESP32 DEVKIT V1 (30-pin)
 * ============================================================================
 */

#include <Arduino.h>

// ============================================================================
// PIN CONFIGURATION (30-PIN ESP32 DEVKIT V1)
// ============================================================================
const int PIN_PUMP       = 25; // 12V Diaphragm Pump MOSFET
const int PIN_VALVE_Z1   = 26; // Zone 1 Valve MOSFET
const int PIN_VALVE_Z2   = 27; // Zone 2 Valve MOSFET
const int PIN_STATUS_LED = 2;  // Onboard Status LED

const int PIN_SENSOR_Z1A = 32; // Zone 1 Probe A (ADC1_CH4)
const int PIN_SENSOR_Z1B = 33; // Zone 1 Probe B (ADC1_CH5)
const int PIN_SENSOR_Z2A = 34; // Zone 2 Probe A (ADC1_CH6)
const int PIN_SENSOR_Z2B = 35; // Zone 2 Probe B (ADC1_CH7)

// ============================================================================
// CALIBRATION & THRESHOLD CONSTANTS
// ============================================================================
// Replace with values from 01_sensor_calibration.ino
const int CAL_Z1A_AIR = 3240, CAL_Z1A_WATER = 1420;
const int CAL_Z1B_AIR = 3215, CAL_Z1B_WATER = 1390;
const int CAL_Z2A_AIR = 3280, CAL_Z2A_WATER = 1450;
const int CAL_Z2B_AIR = 3230, CAL_Z2B_WATER = 1410;

// Moisture Thresholds (%)
const float THRESHOLD_LOW_TRIGGER = 35.0; // Water if moisture drops below this
const float THRESHOLD_HIGH_STOP   = 65.0; // Stop if moisture exceeds this

// Timing Parameters (Milliseconds)
const unsigned long SENSOR_INTERVAL_MS      = 5000;   // Read sensors every 5s
const unsigned long ANTI_HAMMER_DELAY_MS    = 500;    // Valve/pump hydraulic sequencing
const unsigned long MAX_WATERING_RUN_MS     = 45000;  // 45 seconds maximum safety cutoff
const unsigned long SOAK_COOLDOWN_MS        = 180000; // 3 minutes soak cooldown after watering

// ============================================================================
// SYSTEM STATES
// ============================================================================
enum SystemState {
  STATE_IDLE,
  STATE_VALVE_OPENING,
  STATE_PUMP_RUNNING,
  STATE_PUMP_STOPPING,
  STATE_VALVE_CLOSING,
  STATE_COOLDOWN
};

SystemState currentState = STATE_IDLE;
int activeWateringZone = 0; // 0 = None, 1 = Zone 1, 2 = Zone 2
unsigned long stateTimer = 0;
unsigned long wateringStartTimer = 0;
unsigned long lastSensorSampleTime = 0;
unsigned long zone1CooldownUntil = 0;
unsigned long zone2CooldownUntil = 0;

// Filtered moisture values (Exponential Moving Average)
float emaZ1A = 50.0, emaZ1B = 50.0;
float emaZ2A = 50.0, emaZ2B = 50.0;
const float EMA_ALPHA = 0.25;

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================
int sampleADC(int pin) {
  long sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  return (int)(sum / 16);
}

float rawToPercent(int raw, int airVal, int waterVal) {
  if (airVal == waterVal) return 0.0;
  float pct = ((float)(airVal - raw) / (float)(airVal - waterVal)) * 100.0;
  if (pct < 0.0) return 0.0;
  if (pct > 100.0) return 100.0;
  return pct;
}

void updateSensors() {
  int rawZ1A = sampleADC(PIN_SENSOR_Z1A);
  int rawZ1B = sampleADC(PIN_SENSOR_Z1B);
  int rawZ2A = sampleADC(PIN_SENSOR_Z2A);
  int rawZ2B = sampleADC(PIN_SENSOR_Z2B);

  float pctZ1A = rawToPercent(rawZ1A, CAL_Z1A_AIR, CAL_Z1A_WATER);
  float pctZ1B = rawToPercent(rawZ1B, CAL_Z1B_AIR, CAL_Z1B_WATER);
  float pctZ2A = rawToPercent(rawZ2A, CAL_Z2A_AIR, CAL_Z2A_WATER);
  float pctZ2B = rawToPercent(rawZ2B, CAL_Z2B_AIR, CAL_Z2B_WATER);

  // Apply Exponential Moving Average filter
  emaZ1A = (EMA_ALPHA * pctZ1A) + ((1.0 - EMA_ALPHA) * emaZ1A);
  emaZ1B = (EMA_ALPHA * pctZ1B) + ((1.0 - EMA_ALPHA) * emaZ1B);
  emaZ2A = (EMA_ALPHA * pctZ2A) + ((1.0 - EMA_ALPHA) * emaZ2A);
  emaZ2B = (EMA_ALPHA * pctZ2B) + ((1.0 - EMA_ALPHA) * emaZ2B);

  float zone1Avg = (emaZ1A + emaZ1B) / 2.0;
  float zone2Avg = (emaZ2A + emaZ2B) / 2.0;

  Serial.printf("[%7lu s] Z1: %4.1f%% (A:%4.1f%% B:%4.1f%%) | Z2: %4.1f%% (A:%4.1f%% B:%4.1f%%) | State: %d\n",
                millis() / 1000, zone1Avg, emaZ1A, emaZ1B, zone2Avg, emaZ2A, emaZ2B, currentState);
}

void setValve(int zone, bool open) {
  if (zone == 1) {
    digitalWrite(PIN_VALVE_Z1, open ? HIGH : LOW);
  } else if (zone == 2) {
    digitalWrite(PIN_VALVE_Z2, open ? HIGH : LOW);
  }
}

void setPump(bool on) {
  digitalWrite(PIN_PUMP, on ? HIGH : LOW);
  digitalWrite(PIN_STATUS_LED, on ? HIGH : LOW);
}

// ============================================================================
// SETUP & MAIN LOOP
// ============================================================================
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  analogSetAttenuation(ADC_11db);

  pinMode(PIN_PUMP, OUTPUT);
  pinMode(PIN_VALVE_Z1, OUTPUT);
  pinMode(PIN_VALVE_Z2, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);

  // Ensure all actuators are OFF at start
  digitalWrite(PIN_PUMP, LOW);
  digitalWrite(PIN_VALVE_Z1, LOW);
  digitalWrite(PIN_VALVE_Z2, LOW);
  digitalWrite(PIN_STATUS_LED, LOW);

  pinMode(PIN_SENSOR_Z1A, INPUT);
  pinMode(PIN_SENSOR_Z1B, INPUT);
  pinMode(PIN_SENSOR_Z2A, INPUT);
  pinMode(PIN_SENSOR_Z2B, INPUT);

  Serial.println("\n========================================================");
  Serial.println("  ESP32 2-Zone Smart Drip Irrigation Controller Started");
  Serial.println("========================================================");
}

void loop() {
  unsigned long now = millis();

  // Periodic Sensor Sampling
  if (now - lastSensorSampleTime >= SENSOR_INTERVAL_MS) {
    lastSensorSampleTime = now;
    updateSensors();
  }

  float zone1Avg = (emaZ1A + emaZ1B) / 2.0;
  float zone2Avg = (emaZ2A + emaZ2B) / 2.0;

  // Finite State Machine
  switch (currentState) {
    case STATE_IDLE: {
      // Check if Zone 1 needs watering and is not in cooldown
      if (zone1Avg < THRESHOLD_LOW_TRIGGER && now >= zone1CooldownUntil) {
        Serial.println(">>> Zone 1 moisture low! Starting irrigation sequence...");
        activeWateringZone = 1;
        setValve(1, true); // Open Zone 1 Valve first
        stateTimer = now;
        currentState = STATE_VALVE_OPENING;
      }
      // Otherwise check Zone 2
      else if (zone2Avg < THRESHOLD_LOW_TRIGGER && now >= zone2CooldownUntil) {
        Serial.println(">>> Zone 2 moisture low! Starting irrigation sequence...");
        activeWateringZone = 2;
        setValve(2, true); // Open Zone 2 Valve first
        stateTimer = now;
        currentState = STATE_VALVE_OPENING;
      }
      break;
    }

    case STATE_VALVE_OPENING: {
      // Wait for valve to fully open before starting pump (anti-hammer)
      if (now - stateTimer >= ANTI_HAMMER_DELAY_MS) {
        Serial.println(">>> Valve open. Starting diaphragm pump...");
        setPump(true);
        wateringStartTimer = now;
        currentState = STATE_PUMP_RUNNING;
      }
      break;
    }

    case STATE_PUMP_RUNNING: {
      unsigned long wateringElapsed = now - wateringStartTimer;
      float currentZoneAvg = (activeWateringZone == 1) ? zone1Avg : zone2Avg;

      // Stop if moisture target is met or max safety duration exceeded
      if (currentZoneAvg >= THRESHOLD_HIGH_STOP || wateringElapsed >= MAX_WATERING_RUN_MS) {
        Serial.printf(">>> Watering complete for Zone %d (Elapsed: %lu ms, Moisture: %.1f%%). Stopping pump...\n",
                      activeWateringZone, wateringElapsed, currentZoneAvg);
        setPump(false); // Stop pump first!
        stateTimer = now;
        currentState = STATE_PUMP_STOPPING;
      }
      break;
    }

    case STATE_PUMP_STOPPING: {
      // Wait for line pressure to normalize before closing valve
      if (now - stateTimer >= ANTI_HAMMER_DELAY_MS) {
        Serial.printf(">>> Line pressure normalized. Closing Zone %d valve...\n", activeWateringZone);
        setValve(activeWateringZone, false);

        // Set soak cooldown timer
        if (activeWateringZone == 1) {
          zone1CooldownUntil = now + SOAK_COOLDOWN_MS;
        } else {
          zone2CooldownUntil = now + SOAK_COOLDOWN_MS;
        }

        Serial.printf(">>> Zone %d in soil soak cooldown for %lu seconds.\n",
                      activeWateringZone, SOAK_COOLDOWN_MS / 1000);
        activeWateringZone = 0;
        currentState = STATE_IDLE;
      }
      break;
    }

    default:
      currentState = STATE_IDLE;
      break;
  }
}
