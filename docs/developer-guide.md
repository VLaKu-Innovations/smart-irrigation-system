# Developer & Contributing Guide

This guide describes how to configure your development environment, build, flash, test, and extend the ESP32 2-Zone Smart Irrigation System.

---

## 1. Development Environment Setup

You can build and flash the firmware using either **PlatformIO (recommended)** or the standard **Arduino IDE 2.x**.

### Option A: VS Code + PlatformIO (Recommended)
1. Install [Visual Studio Code](https://code.visualstudio.com/).
2. Open VS Code Extensions (`Ctrl+Shift+X` or `Cmd+Shift+X`) and search for **PlatformIO IDE**. Click **Install**.
3. Open this repository folder: `File -> Open Folder... -> smart-irrigation-system`.
4. Open the `firmware/` folder in PlatformIO.
5. Connect your ESP32-WROOM-32 board via USB.
6. Build and upload:
   ```bash
   # From the firmware/ directory
   pio run --target upload
   pio device monitor -b 115200
   ```

### Option B: Arduino IDE 2.x
1. Download and install [Arduino IDE 2.x](https://www.arduino.cc/en/software).
2. Add ESP32 Board Package:
   - Go to `Settings` -> `Additional Boards Manager URLs`.
   - Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. Open `Tools -> Board -> Boards Manager`, search for **esp32**, and install **esp32 by Espressif Systems**.
4. Select `Tools -> Board -> ESP32 Arduino -> DOIT ESP32 DEVKIT V1`.
5. Select the appropriate serial port under `Tools -> Port`.
6. Open any sample sketch (e.g., `samples/03_dual_zone_standalone/03_dual_zone_standalone.ino`) and click **Upload**.

---

## 2. Firmware Architecture & Source Code Layout

The production firmware located in `firmware/` is organized into clean, reusable C++ classes:

```
firmware/
├── platformio.ini              # Board configuration & compiler flags
├── include/
│   ├── config.h                # All pinouts, calibration constants & timing limits
│   ├── soil_sensor.h           # SoilSensor class (ADC oversampling, EMA filter, % calculation)
│   ├── actuator_driver.h       # ActuatorDriver class (MOSFET gate control, watchdog timeout)
│   ├── zone_manager.h          # ZoneManager class (manages 2 sensors + 1 valve per zone)
│   └── irrigation_system.h     # IrrigationSystem class (coordinates pump & zone sequencing)
└── src/
    ├── main.cpp                # Setup, status LED heartbeat, and master run loop
    ├── soil_sensor.cpp
    ├── actuator_driver.cpp
    ├── zone_manager.cpp
    └── irrigation_system.cpp
```

### Class Responsibilities
1. **`SoilSensor`**:
   - Performs 16-sample analog reads to smooth noise.
   - Applies Exponential Moving Average (`EMA_ALPHA = 0.20`).
   - Converts raw ADC values into percentage moisture based on dry/wet calibration points.
   - Detects disconnected or shorted sensor hardware faults.
2. **`ActuatorDriver`**:
   - Controls MOSFET gate pins with active-HIGH logic.
   - Enforces an internal safety timer (auto-shuts off if ON time exceeds `maxDurationMs`).
   - Tracks cumulative on-time and cycle counts.
3. **`ZoneManager`**:
   - Associates two `SoilSensor` instances with one `ActuatorDriver` valve.
   - Calculates the average moisture level between sensors.
   - Determines if the zone needs water based on low/high threshold hysteresis.
   - Enforces a mandatory soil soak cooldown period after watering.
4. **`IrrigationSystem`**:
   - Coordinates the single 12V diaphragm pump and the two zone managers.
   - Implements anti-water-hammer sequencing (valve opens before pump starts; pump stops before valve closes).
   - Guarantees only one zone waters at a time to prevent overloading the 12V 5A power supply.

---

## 3. Coding Guidelines

1. **Non-Blocking Architecture**:
   - Never call `delay()` inside loops. Use `millis()`-based timestamps for state checks and timing.
2. **ADC1 Exclusivity**:
   - Only assign analog inputs to ADC1 pins (`GPIO 32, 33, 34, 35`).
3. **Hardware Watchdogs**:
   - Always ensure any actuator driver has a maximum continuous runtime safety cutoff.
4. **Descriptive Telemetry**:
   - Output structured, human-readable status logs over Serial (115200 baud).

---

## 4. Extending the System

### Adding Wi-Fi & MQTT (Home Assistant Integration)
The pin layout was deliberately chosen to leave Wi-Fi, SPI, and I2C completely free:
- `GPIO 21 (SDA)` and `GPIO 22 (SCL)` are available for I2C (e.g., an OLED display or BME280 weather sensor).
- ADC1 does not conflict with the ESP32 Wi-Fi radio.
- To publish sensor data to MQTT:
  1. Add `knolleary/PubSubClient` to `firmware/platformio.ini`.
  2. In `main.cpp`, add Wi-Fi connection logic in a non-blocking background task.
  3. Publish zone moisture percentages and valve/pump states in JSON format to your MQTT broker every 10 seconds.

### Adding a Water Reservoir Float Switch
To protect the diaphragm pump from running dry when the reservoir is empty:
1. Connect a digital reed float switch between `GPIO 18` and `GND`.
2. Enable internal pull-up: `pinMode(FLOAT_SWITCH_PIN, INPUT_PULLUP)`.
3. In `irrigation_system.cpp`, check the float switch state before starting the pump. If LOW (empty), abort the cycle and log an error.
