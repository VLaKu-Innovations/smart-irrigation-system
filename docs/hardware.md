# Hardware & Electrical Specification

This document details the hardware components, electrical characteristics, power budgeting, and circuit protection rules for the ESP32 2-Zone Smart Drip Irrigation System.

---

## 1. Complete Bill of Materials (BOM)

| Component | Specification | Qty | Key Ratings | Purpose |
| :--- | :--- | :---: | :--- | :--- |
| **ESP32 DevKit V1** | ESP32-WROOM-32 (30-pin) | 1 | Dual core 240MHz, 520KB SRAM, 3.3V logic | System brain, ADC sampling, valve/pump control |
| **Diaphragm Pump** | 12V DC self-priming | 1 | 12V, 3–5A peak, ~5 L/min (80 PSI cutoff) | Delivers water from tank to drip network |
| **Zone Solenoid Valves** | 12V DC Normally Closed (NC) | 2 | 12V, 300–450mA (~5W), 1/2" NPT/BSP threads | Zone isolation (Zone 1 & Zone 2) |
| **Soil Moisture Sensors**| Capacitive v1.2 / v2.0 | 4 | 3.3V–5V VCC, analog voltage output (corrosion-free) | Soil moisture measurement (2 per zone) |
| **Power Supply** | 12V 5A DC Wall Adapter | 1 | 12V DC, 60W regulated | Main power source for pump, valves, and ESP32 |
| **Buck Converter** | DC-DC Step-Down (e.g. LM2596/MP1584)| 1 | Input 7–28V, Output 5.0V @ 3A max | Steps down 12V to clean 5V for ESP32 VIN |
| **MOSFET Drivers** | Logic-Level N-Channel (e.g. LR7843 / IRLZ44N) | 3 | $V_{GS(th)} \le 2.5V$, $I_D \ge 10A$, opto-isolated or direct | Low-side switching of Pump and 2 Valves |
| **Flyback Diodes** | 1N4007 Silicon Rectifier | 3 | 1000V PRV, 1A continuous, 30A surge | Shunts back-EMF spikes from pump and coils |
| **Main Irrigation Pipe** | 16mm LDPE / HDPE pipe | 25m | UV-resistant, 4 bar rating | Main distribution line to beds/zones |
| **Feeder Microtube** | 4mm Drip Tubing | 25m | Vinyl / PE, flexible | Connects mainline to individual plants |
| **Drip Emitters** | Pressure-Compensating (PC) | 20 | 2.0 Liters Per Hour (LPH), 0.5–3.5 bar | Constant drip rate regardless of elevation/length |
| **Irrigation Filter** | 120-Mesh (130 Micron) Disc/Screen | 1 | 1/2" male/female threads | Protects pump valves and emitters from dirt |
| **Manifold** | 2-Way 1/2" Ball / Solenoid Manifold | 1 | 1/2" inlet to dual 1/2" outlets | Splits filtered pump output to Zones 1 and 2 |
| **Fittings & Clamps** | 16mm barbs, tees, elbows, end plugs | 1 set | Barbed with ratchet/worm-drive clamps | Leak-free connections |

---

## 2. Power Budget Analysis

All components are powered from a single **12V 5A (60W)** power brick. The power budget is designed with strict margin:

| Device | Operating Voltage | Nominal Current | Peak Inrush Current | Power Consumption |
| :--- | :--- | :--- | :--- | :--- |
| **Diaphragm Pump** | 12V DC | 2.8 A | 4.5 A (start-up) | 33.6 W (Nominal) |
| **Solenoid Valve 1** | 12V DC | 0.35 A | 0.45 A | 4.2 W |
| **Solenoid Valve 2** | 12V DC | 0.35 A | 0.45 A | 4.2 W |
| **ESP32 + 4 Sensors** | 5V (via Buck) | 0.25 A (@ 5V) | 0.40 A (@ 5V) | ~1.5 W (@ 12V $\approx$ 0.15A) |
| **Total (1 Zone Active)**| 12V DC | **3.30 A** | **5.00 A (instantaneous)** | **~39.5 W** |
| **Total (Both Zones Active)**| 12V DC | **3.65 A** | **5.45 A (rare/avoided)** | **~43.8 W** |

> [!TIP]
> **Sequential Watering Optimization**:
> The firmware activates **only one zone at a time**. This guarantees total steady-state draw remains around **3.3A**, well within the continuous 5.0A rating of the power adapter, preventing thermal stress and voltage sag.

---

## 3. MOSFET Driver Selection & Gate Control

Standard power MOSFETs (like the classic IRF540N) require a Gate-to-Source voltage ($V_{GS}$) of **10V** to fully saturate. Driving them directly with the ESP32's **3.3V GPIO** will leave them in the linear region, causing extreme heat dissipation and failure.

### Required Driver Characteristics
1. **Logic-Level Gate**: Must have a threshold voltage $V_{GS(th)} \le 2.0\text{V}$ and low $R_{DS(on)}$ at $V_{GS} = 3.3\text{V}$.
   - Recommended: **LR7843**, **IRLZ44N**, or breakout boards with built-in optocoupler and gate drive transistor.
2. **10 kΩ Gate Pull-Down Resistor**:
   - During ESP32 boot, GPIO pins float in high-impedance mode (Hi-Z). Without a pull-down resistor, the MOSFET gate can drift high and turn the pump or valve ON unexpectedly.
   - Install a **10 kΩ resistor between Gate and Source (GND)** on every driver module.

---

## 4. Inductive Kickback & Flyback Diode (1N4007)

Both the **diaphragm pump motor** and the **solenoid valve coils** are inductive loads ($V = -L \frac{di}{dt}$). When the MOSFET turns OFF, the magnetic field collapses, inducing a reverse voltage spike exceeding **100V to 300V**.

### Protection Circuit
- Solder a **1N4007 diode** directly across the actuator leads:
  - **Cathode (Silver Band)**: Connects to **+12V Supply Rail**.
  - **Anode**: Connects to **MOSFET Drain / Switched Ground**.
- Under normal operation, the diode is reverse-biased (no current flows).
- When the MOSFET turns OFF, the back-EMF spike forward-biases the diode, harmlessly recirculating the current through the coil until dissipated.

---

## 5. Soil Moisture Sensor Integration

Capacitive soil moisture sensors (v1.2 or v2.0) measure the dielectric permittivity of the surrounding medium using a high-frequency TLC555 timer oscillator circuit:
1. **Corrosion Resistance**: Unlike cheap resistive probes that corrode within weeks, capacitive sensors have no exposed copper in the soil.
2. **Supply Voltage**: Connect sensor VCC to the **ESP32 3.3V pin**.
   - *Crucial*: Powering the sensor with 5V will output an analog signal up to 4.2V, which exceeds the absolute maximum 3.6V rating of ESP32 GPIOs and will destroy the ADC!
3. **ADC Pin Assignment**:
   - The ESP32 has two ADCs: **ADC1** (GPIO 32–39) and **ADC2** (GPIO 0, 2, 4, 12–15, 25–27).
   - **ADC2 cannot be used when Wi-Fi is active**.
   - All 4 sensors are mapped to **ADC1**:
     - `GPIO 32` (Zone 1 - Sensor A)
     - `GPIO 33` (Zone 1 - Sensor B)
     - `GPIO 34` (Zone 2 - Sensor A, input only)
     - `GPIO 35` (Zone 2 - Sensor B, input only)

---

## 6. Buck Converter Tuning & Calibration

Before connecting the DC-DC buck converter to the ESP32:
1. Connect the buck converter input to the 12V 5A adapter.
2. Use a digital multimeter across the buck converter's `OUT+` and `OUT-`.
3. Adjust the multi-turn trimpot until the output voltage reads **5.05V – 5.10V** (never exceed 5.25V).
4. Power off, connect `OUT+` to ESP32 **VIN**, and `OUT-` to common **GND**.
