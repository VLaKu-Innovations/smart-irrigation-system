/**
 * ============================================================================
 * Project: ESP32 2-Zone Smart Drip Irrigation System
 * File: main.cpp
 * Description: Production entry point for modular 2-zone drip irrigation controller.
 * Board: DOIT ESP32 DEVKIT V1 (30-pin)
 * ============================================================================
 */

#include <Arduino.h>
#include "config.h"
#include "soil_sensor.h"
#include "actuator_driver.h"
#include "zone_manager.h"
#include "irrigation_system.h"

// ----------------------------------------------------------------------------
// SENSORS & ACTUATORS INSTANTIATION
// ----------------------------------------------------------------------------
// Capacitive Soil Moisture Sensors (ADC1: Pins 32, 33, 34, 35)
SoilSensor sensorZ1A(PIN_SENSOR_Z1_A, CALIB_Z1_A_AIR, CALIB_Z1_A_WATER, "Zone 1 - Sensor A");
SoilSensor sensorZ1B(PIN_SENSOR_Z1_B, CALIB_Z1_B_AIR, CALIB_Z1_B_WATER, "Zone 1 - Sensor B");
SoilSensor sensorZ2A(PIN_SENSOR_Z2_A, CALIB_Z2_A_AIR, CALIB_Z2_A_WATER, "Zone 2 - Sensor A");
SoilSensor sensorZ2B(PIN_SENSOR_Z2_B, CALIB_Z2_B_AIR, CALIB_Z2_B_WATER, "Zone 2 - Sensor B");

// MOSFET Actuators (Pins 25, 26, 27)
ActuatorDriver pumpDriver(PIN_MOSFET_PUMP, "12V Diaphragm Pump", MAX_WATERING_RUN_MS);
ActuatorDriver valve1Driver(PIN_MOSFET_VALVE_1, "Zone 1 Solenoid Valve", MAX_WATERING_RUN_MS + 2000UL);
ActuatorDriver valve2Driver(PIN_MOSFET_VALVE_2, "Zone 2 Solenoid Valve", MAX_WATERING_RUN_MS + 2000UL);

// Zone Managers (Couples 2 sensors + 1 valve)
ZoneManager zone1(1, sensorZ1A, sensorZ1B, valve1Driver);
ZoneManager zone2(2, sensorZ2A, sensorZ2B, valve2Driver);

// Master Orchestrator
IrrigationSystem irrigation(pumpDriver, zone1, zone2);

// Heartbeat status tracking
unsigned long lastHeartbeatMs = 0;
bool heartbeatState = false;

// ----------------------------------------------------------------------------
// SETUP
// ----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  // Set ADC attenuation to 11dB for 0 to ~3.1V input range
  analogSetAttenuation(ADC_11db);

  // Configure Status LED
  pinMode(PIN_STATUS_HEARTBEAT, OUTPUT);
  digitalWrite(PIN_STATUS_HEARTBEAT, LOW);

  Serial.println("\n========================================================");
  Serial.println("  ESP32 2-Zone Smart Drip Irrigation System (Modular)");
  Serial.println("========================================================");
  Serial.println("Board: DOIT ESP32 DEVKIT V1 (30-pin)");
  Serial.println("Firmware: v1.0.0 (Anti-Hammer & Dual-Sensor Consensus)");
  Serial.println("Initializing hardware drivers...\n");

  irrigation.begin();

  Serial.println("[SYSTEM] Hardware initialization complete. System RUNNING.\n");
}

// ----------------------------------------------------------------------------
// MAIN LOOP
// ----------------------------------------------------------------------------
void loop() {
  // Update state machine and drivers
  irrigation.update();

  // Non-blocking Heartbeat Blink
  unsigned long now = millis();
  if (now - lastHeartbeatMs >= HEARTBEAT_BLINK_MS) {
    lastHeartbeatMs = now;
    heartbeatState = !heartbeatState;
    digitalWrite(PIN_STATUS_HEARTBEAT, heartbeatState ? HIGH : LOW);
  }
}
