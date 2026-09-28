# Capacitive Soil Moisture Sensor Calibration Guide

This guide walks you through calibrating the **Capacitive Soil Moisture Sensors (v1.2 / v2.0)** on the ESP32 to achieve reliable, accurate moisture percentages (0%–100%).

---

## 1. Operating Principle & The Inverse Response

Unlike resistive soil sensors that pass DC current between exposed metal prongs (causing rapid electrochemical corrosion within weeks), capacitive sensors use high-frequency oscillation (typically a 555-timer IC at ~1.5 MHz) to measure the **dielectric permittivity** of the soil:

- **Dry Soil / Air**: Low dielectric constant ($\approx 1$). Low capacitance $\rightarrow$ **High Output Voltage $\rightarrow$ High ADC Value (~3200)**.
- **Saturated Soil / Water**: Water has a high dielectric constant ($\approx 80$). High capacitance $\rightarrow$ **Low Output Voltage $\rightarrow$ Low ADC Value (~1400)**.

> [!IMPORTANT]
> **Inverse Value Alert**:
> A **higher** ADC reading indicates **drier** soil. A **lower** ADC reading indicates **wetter** soil.

---

## 2. ESP32 ADC Characteristics & Best Practices

1. **Resolution**: The ESP32 ADC has 12-bit resolution ($2^{12} = 4096$ steps, output 0 to 4095).
2. **Attenuation**: Set analog attenuation to `ADC_11db` (default in Arduino ESP32 core) to enable reading voltages up to ~3.1V.
3. **ADC1 Exclusivity**: Always use ADC1 pins (`GPIO 32, 33, 34, 35`). Do not use ADC2 pins if Wi-Fi or Bluetooth will ever be used.
4. **Oversampling**: Raw ADC readings can fluctuate due to RF noise. The firmware samples 16 to 32 readings in quick succession and calculates the trimmed average.

---

## 3. Two-Point Calibration Procedure

You will measure two points for each of the 4 sensors: **Air (0% moisture)** and **Water (100% moisture)**.

### Preparation
1. Connect all 4 sensors to the ESP32:
   - VCC $\rightarrow$ ESP32 **3V3**
   - GND $\rightarrow$ ESP32 **GND**
   - Signal outputs $\rightarrow$ **GPIO 32, 33, 34, 35**
2. Flash the calibration sketch: [`samples/01_sensor_calibration/01_sensor_calibration.ino`](../samples/01_sensor_calibration/01_sensor_calibration.ino).
3. Open the Serial Monitor at **115200 baud**.
4. Prepare a glass of tap water and paper towels.

---

### Step 1: Measuring Air Value (0% Moisture - `AIR_VALUE`)
1. Ensure the sensor probe is completely clean and dry.
2. Hold the probe by the top edge in ambient room air (do not touch the sensor prongs with your fingers, as skin capacitance skews the reading).
3. Observe the raw ADC readings on the Serial Monitor for 10–15 seconds.
4. Record the average reading as `AIR_VALUE` for that sensor (typical range: **3100 to 3350**).

---

### Step 2: Measuring Water Value (100% Moisture - `WATER_VALUE`)
1. Dip the sensor blade into the glass of water.
   > [!CAUTION]
   > **Do Not Submerge Electronics**:
   > Submerge only the lower white-lined PCB blade up to the marked line. **Never submerge the top circuit board**, components, or wire connector in water!
2. Gently stir to remove air bubbles adhering to the probe surface.
3. Observe the Serial Monitor and record the stable minimum reading as `WATER_VALUE` (typical range: **1250 to 1550**).
4. Remove the probe, dry thoroughly with a paper towel, and repeat for the other 3 sensors.

---

## 4. Calibration Data Sheet & Configuration

Record your readings in a table like the following:

| Sensor ID | GPIO Pin | Zone / Position | Dry Air ADC (`AIR_VAL`) | Pure Water ADC (`WATER_VAL`) |
| :--- | :--- | :--- | :---: | :---: |
| **Sensor 1** | `GPIO 32` | Zone 1 - Root Depth A | 3240 | 1420 |
| **Sensor 2** | `GPIO 33` | Zone 1 - Root Depth B | 3215 | 1390 |
| **Sensor 3** | `GPIO 34` | Zone 2 - Root Depth A | 3280 | 1450 |
| **Sensor 4** | `GPIO 35` | Zone 2 - Root Depth B | 3230 | 1410 |

Update these values in [`firmware/include/config.h`](../firmware/include/config.h):

```cpp
// Calibration constants for Zone 1
const int SENSOR_Z1A_AIR   = 3240;
const int SENSOR_Z1A_WATER = 1420;
const int SENSOR_Z1B_AIR   = 3215;
const int SENSOR_Z1B_WATER = 1390;

// Calibration constants for Zone 2
const int SENSOR_Z2A_AIR   = 3280;
const int SENSOR_Z2A_WATER = 1450;
const int SENSOR_Z2B_AIR   = 3230;
const int SENSOR_Z2B_WATER = 1410;
```

---

## 5. Moisture Conversion Formula

The calibrated moisture percentage is calculated as:

$$\text{Moisture (\%)} = \left( \frac{\text{AIR\_VAL} - \text{Raw\_ADC}}{\text{AIR\_VAL} - \text{WATER\_VAL}} \right) \times 100.0$$

The firmware automatically clamps the output between **0.0%** and **100.0%** and filters out electrical spikes using an Exponential Moving Average (EMA).
