/**
 * ============================================================================
 * Project: ESP32 2-Zone Smart Drip Irrigation System
 * File: 02_mosfet_actuator_test.ino
 * Description: Diagnostic test for MOSFET drivers, 12V pump, and solenoid valves.
 *              Executes strict anti-water-hammer sequencing:
 *              Valve Open -> 500ms -> Pump ON -> Pump OFF -> 500ms -> Valve Close.
 * Board: DOIT ESP32 DEVKIT V1 (30-pin)
 * ============================================================================
 */

#include <Arduino.h>

// Actuator Pin Definitions (Logic-level MOSFET gates)
const int PIN_MOSFET_PUMP    = 25; // GPIO 25 (12V Diaphragm Pump)
const int PIN_MOSFET_VALVE_1 = 26; // GPIO 26 (Zone 1 Solenoid Valve)
const int PIN_MOSFET_VALVE_2 = 27; // GPIO 27 (Zone 2 Solenoid Valve)
const int PIN_STATUS_LED     = 2;  // GPIO 2  (Onboard Blue LED)

// Timing Configuration
const unsigned long ANTI_HAMMER_DELAY_MS = 500;  // Pre/post pump buffer
const unsigned long TEST_WATERING_RUN_MS = 3000; // 3 seconds test pulse
const unsigned long INTER_ZONE_PAUSE_MS  = 5000; // 5 seconds pause between tests

void setActuator(int pin, bool state, const char* name) {
  digitalWrite(pin, state ? HIGH : LOW);
  Serial.printf("[%9lu ms] Actuator: %-16s --> %s\n", millis(), name, state ? "ON / OPEN" : "OFF / CLOSED");
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  // Initialize GPIO pins as OUTPUT and set LOW immediately
  pinMode(PIN_MOSFET_PUMP, OUTPUT);
  pinMode(PIN_MOSFET_VALVE_1, OUTPUT);
  pinMode(PIN_MOSFET_VALVE_2, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);

  digitalWrite(PIN_MOSFET_PUMP, LOW);
  digitalWrite(PIN_MOSFET_VALVE_1, LOW);
  digitalWrite(PIN_MOSFET_VALVE_2, LOW);
  digitalWrite(PIN_STATUS_LED, LOW);

  Serial.println("\n============================================================");
  Serial.println("  ESP32 Actuator & Hydraulic Sequencing Verification Test");
  Serial.println("============================================================");
  Serial.println("Safety Notice:");
  Serial.println("1. Verify 1N4007 flyback diodes are installed across pump & valves.");
  Serial.println("2. Confirm 12V 5A power supply is connected.");
  Serial.println("3. Ensure water reservoir has adequate water before starting.");
  Serial.println("------------------------------------------------------------\n");
  delay(3000);
}

void testZone(int valvePin, const char* zoneName) {
  Serial.printf("\n--- Starting Test Sequence for: %s ---\n", zoneName);
  digitalWrite(PIN_STATUS_LED, HIGH);

  // Step 1: Open Solenoid Valve FIRST
  Serial.println("Step 1: Opening Zone Solenoid Valve...");
  setActuator(valvePin, true, zoneName);

  // Step 2: Hydraulic anti-hammer delay
  Serial.printf("Step 2: Waiting %lu ms for valve to fully open...\n", ANTI_HAMMER_DELAY_MS);
  delay(ANTI_HAMMER_DELAY_MS);

  // Step 3: Start 12V Diaphragm Pump
  Serial.println("Step 3: Starting Diaphragm Pump...");
  setActuator(PIN_MOSFET_PUMP, true, "12V Diaphragm Pump");

  // Step 4: Run water flow for test duration
  Serial.printf("Step 4: Running flow for %lu ms...\n", TEST_WATERING_RUN_MS);
  delay(TEST_WATERING_RUN_MS);

  // Step 5: Stop Pump FIRST
  Serial.println("Step 5: Stopping Diaphragm Pump...");
  setActuator(PIN_MOSFET_PUMP, false, "12V Diaphragm Pump");

  // Step 6: Anti-hammer pressure relief delay
  Serial.printf("Step 6: Waiting %lu ms for line pressure to normalize...\n", ANTI_HAMMER_DELAY_MS);
  delay(ANTI_HAMMER_DELAY_MS);

  // Step 7: Close Solenoid Valve
  Serial.println("Step 7: Closing Zone Solenoid Valve...");
  setActuator(valvePin, false, zoneName);

  digitalWrite(PIN_STATUS_LED, LOW);
  Serial.printf("--- Test Sequence for %s Completed Successfully ---\n\n", zoneName);
}

void loop() {
  // Test Zone 1
  testZone(PIN_MOSFET_VALVE_1, "Zone 1 Valve (GPIO 26)");
  
  Serial.printf("Pausing %lu ms before next zone test...\n", INTER_ZONE_PAUSE_MS);
  delay(INTER_ZONE_PAUSE_MS);

  // Test Zone 2
  testZone(PIN_MOSFET_VALVE_2, "Zone 2 Valve (GPIO 27)");

  Serial.println("All zones tested. Waiting 15 seconds before repeating cycle...\n");
  delay(15000);
}
