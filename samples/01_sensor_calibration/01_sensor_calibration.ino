/**
 * ============================================================================
 * Project: ESP32 2-Zone Smart Drip Irrigation System
 * File: 01_sensor_calibration.ino
 * Description: Interactive calibration tool for 4x Capacitive Soil Moisture Sensors.
 *              Measures raw 12-bit ADC values, calculates voltage, and displays
 *              dry/water reference values on Serial Monitor (115200 baud).
 * Board: DOIT ESP32 DEVKIT V1 (30-pin)
 * ============================================================================
 */

#include <Arduino.h>

// Sensor Pin Definitions (All on ADC1)
const int PIN_SENSOR_Z1A = 32; // Zone 1 - Sensor A (ADC1_CH4)
const int PIN_SENSOR_Z1B = 33; // Zone 1 - Sensor B (ADC1_CH5)
const int PIN_SENSOR_Z2A = 34; // Zone 2 - Sensor A (ADC1_CH6)
const int PIN_SENSOR_Z2B = 35; // Zone 2 - Sensor B (ADC1_CH7)

// Number of samples to average per burst
const int NUM_SAMPLES = 20;

// Default provisional calibration points (Dry Air vs Submerged in Water)
// Update these with the values obtained during this calibration process!
int airValZ1A = 3250, waterValZ1A = 1400;
int airValZ1B = 3250, waterValZ1B = 1400;
int airValZ2A = 3250, waterValZ2A = 1400;
int airValZ2B = 3250, waterValZ2B = 1400;

/**
 * Reads analog pin with oversampling to filter out electrical and RF noise.
 */
int readAveragedADC(int pin) {
  long sum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    sum += analogRead(pin);
    delayMicroseconds(500);
  }
  return (int)(sum / NUM_SAMPLES);
}

/**
 * Converts raw ADC reading into moisture percentage based on two-point calibration.
 */
float calculateMoisturePercent(int raw, int airVal, int waterVal) {
  if (airVal == waterVal) return 0.0;
  float pct = ((float)(airVal - raw) / (float)(airVal - waterVal)) * 100.0;
  if (pct < 0.0) pct = 0.0;
  if (pct > 100.0) pct = 100.0;
  return pct;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for serial console

  // Configure ADC attenuation (0 to ~3.1V range)
  analogSetAttenuation(ADC_11db);

  // Configure input pins
  pinMode(PIN_SENSOR_Z1A, INPUT);
  pinMode(PIN_SENSOR_Z1B, INPUT);
  pinMode(PIN_SENSOR_Z2A, INPUT);
  pinMode(PIN_SENSOR_Z2B, INPUT);

  Serial.println("\n========================================================");
  Serial.println("  ESP32 4-Sensor Capacitive Calibration Monitor");
  Serial.println("========================================================");
  Serial.println("Instructions:");
  Serial.println("1. Dry Air Measurement: Hold sensor dry in ambient air.");
  Serial.println("   Record the Raw ADC value as AIR_VAL (~3100 - 3350).");
  Serial.println("2. Water Measurement: Dip probe blade into a cup of water");
  Serial.println("   (DO NOT wet the top electronic circuitry!).");
  Serial.println("   Record the Raw ADC value as WATER_VAL (~1300 - 1550).");
  Serial.println("--------------------------------------------------------\n");
  delay(2000);
}

void loop() {
  // Read all 4 sensors
  int rawZ1A = readAveragedADC(PIN_SENSOR_Z1A);
  int rawZ1B = readAveragedADC(PIN_SENSOR_Z1B);
  int rawZ2A = readAveragedADC(PIN_SENSOR_Z2A);
  int rawZ2B = readAveragedADC(PIN_SENSOR_Z2B);

  // Convert to voltage (3.3V / 4095 steps)
  float voltZ1A = (rawZ1A * 3.3) / 4095.0;
  float voltZ1B = (rawZ1B * 3.3) / 4095.0;
  float voltZ2A = (rawZ2A * 3.3) / 4095.0;
  float voltZ2B = (rawZ2B * 3.3) / 4095.0;

  // Calculate estimated moisture percentages
  float pctZ1A = calculateMoisturePercent(rawZ1A, airValZ1A, waterValZ1A);
  float pctZ1B = calculateMoisturePercent(rawZ1B, airValZ1B, waterValZ1B);
  float pctZ2A = calculateMoisturePercent(rawZ2A, airValZ2A, waterValZ2A);
  float pctZ2B = calculateMoisturePercent(rawZ2B, airValZ2B, waterValZ2B);

  // Output formatted diagnostic table
  Serial.println("---------------------------------------------------------------------");
  Serial.println("Sensor ID        | Pin     | Raw ADC (12-bit) | Voltage (V) | Moisture (%)");
  Serial.println("---------------------------------------------------------------------");

  Serial.printf("Zone 1 - Probe A | GPIO 32 | %-16d | %-11.2f | %5.1f %%\n", rawZ1A, voltZ1A, pctZ1A);
  Serial.printf("Zone 1 - Probe B | GPIO 33 | %-16d | %-11.2f | %5.1f %%\n", rawZ1B, voltZ1B, pctZ1B);
  Serial.printf("Zone 2 - Probe A | GPIO 34 | %-16d | %-11.2f | %5.1f %%\n", rawZ2A, voltZ2A, pctZ2A);
  Serial.printf("Zone 2 - Probe B | GPIO 35 | %-16d | %-11.2f | %5.1f %%\n", rawZ2B, voltZ2B, pctZ2B);
  Serial.println("---------------------------------------------------------------------\n");

  delay(1500);
}
