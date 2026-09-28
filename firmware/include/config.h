#pragma once

#include <Arduino.h>

/**
 * ============================================================================
 * Hardware Pinout Configuration (ESP32-WROOM-32 30-Pin DevKit V1)
 * ============================================================================
 */

// MOSFET Driver Output Pins (Low-side switching with 10k gate pull-down)
constexpr int PIN_MOSFET_PUMP    = 25; // 12V DC Diaphragm Pump
constexpr int PIN_MOSFET_VALVE_1 = 26; // Zone 1 Solenoid Valve (12V NC)
constexpr int PIN_MOSFET_VALVE_2 = 27; // Zone 2 Solenoid Valve (12V NC)
constexpr int PIN_STATUS_HEARTBEAT = 2; // Onboard Status Blue LED

// Capacitive Soil Moisture Sensor Analog Inputs (Strictly ADC1 pins)
constexpr int PIN_SENSOR_Z1_A = 32; // Zone 1 - Sensor A (ADC1_CH4)
constexpr int PIN_SENSOR_Z1_B = 33; // Zone 1 - Sensor B (ADC1_CH5)
constexpr int PIN_SENSOR_Z2_A = 34; // Zone 2 - Sensor A (ADC1_CH6, input only)
constexpr int PIN_SENSOR_Z2_B = 35; // Zone 2 - Sensor B (ADC1_CH7, input only)

/**
 * ============================================================================
 * Calibration Defaults (Raw 12-Bit ADC values: 0 to 4095)
 * Update these with your values from 01_sensor_calibration.ino!
 * ============================================================================
 */
constexpr int CALIB_Z1_A_AIR   = 3240;
constexpr int CALIB_Z1_A_WATER = 1420;

constexpr int CALIB_Z1_B_AIR   = 3215;
constexpr int CALIB_Z1_B_WATER = 1390;

constexpr int CALIB_Z2_A_AIR   = 3280;
constexpr int CALIB_Z2_A_WATER = 1450;

constexpr int CALIB_Z2_B_AIR   = 3230;
constexpr int CALIB_Z2_B_WATER = 1410;

// Fault limits: ADC outside this range is considered shorted or disconnected
constexpr int SENSOR_FAULT_MIN_ADC = 800;  // Below this = short to GND or saturated
constexpr int SENSOR_FAULT_MAX_ADC = 3900; // Above this = open circuit / disconnected

/**
 * ============================================================================
 * Irrigation Control Parameters
 * ============================================================================
 */
constexpr float MOISTURE_TRIGGER_LOW_PCT = 35.0f; // Start watering when zone moisture < 35%
constexpr float MOISTURE_SATISFIED_HIGH_PCT = 65.0f; // Stop watering when zone moisture >= 65%

// Non-blocking timing settings (Milliseconds)
constexpr unsigned long SENSOR_SAMPLE_INTERVAL_MS = 5000UL;   // Read sensors every 5s
constexpr unsigned long ANTI_HAMMER_DELAY_MS      = 500UL;    // Delay between valve and pump
constexpr unsigned long MAX_WATERING_RUN_MS       = 60000UL;  // 60s hard safety shutoff
constexpr unsigned long SOAK_COOLDOWN_MS          = 300000UL; // 5 min soak delay before re-check
constexpr unsigned long HEARTBEAT_BLINK_MS        = 1000UL;   // LED blink interval

// Filter Settings
constexpr int   OVERSAMPLE_BURST_COUNT = 16;
constexpr float SENSOR_EMA_ALPHA       = 0.20f; // Exponential Moving Average smoothing factor
