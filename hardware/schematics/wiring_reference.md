# Wiring Reference & Circuit Schematics

This document provides terminal-friendly ASCII schematics, pinout tables, and wiring instructions for the 30-Pin ESP32-WROOM-32 Smart Irrigation System.

---

## 1. ESP32 30-Pin Header Pinout Table

```
                         ESP32 DevKit V1 (30 Pins)
                             ┌──────────────┐
                    EN ──[ 1]│              │[30]── 3V3  ──► Power 4x Soil Sensors (3.3V)
                    VP ──[ 2]│              │[29]── GND  ──► Common Ground Bus
                    VN ──[ 3]│              │[28]── D15
    Zone 2 Sensor A ──► D34 ──[ 4]│              │[27]── D2   ──► Onboard Status LED
    Zone 2 Sensor B ──► D35 ──[ 5]│              │[26]── D4
    Zone 1 Sensor A ──► D32 ──[ 6]│              │[25]── RX2
    Zone 1 Sensor B ──► D33 ──[ 7]│              │[24]── TX2
    Pump MOSFET Gate ──► D25 ──[ 8]│              │[23]── D5
 Zone 1 Valve Gate ──► D26 ──[ 9]│              │[22]── D18
 Zone 2 Valve Gate ──► D27 ──[10]│              │[21]── D19
                   D14 ──[11]│              │[20]── D21  (I2C SDA)
                   D12 ──[12]│              │[19]── RX0
                   D13 ──[13]│              │[18]── TX0
     Common Ground ◄── GND ──[14]│              │[17]── D22  (I2C SCL)
  +5V from Buck In ──► VIN ──[15]│  [MicroUSB]  │[16]── D23
                             └──────────────┘
```

---

## 2. Power Distribution Network

```
+12V 5A Adapter
  │
  ├───[+] 12V ────────────────────────┬──────────────┬──────────────┐
  │                                   │              │              │
  │                                   ▼              ▼              ▼
  │                             Pump Coil (+)  Valve 1 (+)    Valve 2 (+)
  │                                   │              │              │
  │                             [1N4007 Diode] [1N4007 Diode] [1N4007 Diode]
  │                             (Cathode to +) (Cathode to +) (Cathode to +)
  │                                   │              │              │
  │                                   ▼              ▼              ▼
  │                              MOSFET 1       MOSFET 2       MOSFET 3
  │                               (Drain)        (Drain)        (Drain)
  │
  └───[+] 12V IN ──► [ DC-DC Buck Converter ] ──► +5.0V OUT ──► ESP32 VIN (Pin 15)
                             │                                       │
                             │ (Common GND)                          ▼
                             │                                 ESP32 3V3 (Pin 30)
                             │                                       │
                             │                                       ▼
                             │                                 VCC for 4x Sensors
                             │
  COMMON GROUND BUS ─────────┴───────────────────────────────────────┴──► All GND Pins
```

---

## 3. MOSFET Driver & Flyback Diode Connection (x3 Modules)

For each load (1x Diaphragm Pump, 2x Solenoid Valves):

```
                       +12V Supply Rail
                              │
                    ┌─────────┴─────────┐
                    │                   │
                    │               ┌───┴───┐
                    │               │ 1N4007│ (Silver band points to +12V)
                    ▼               │ DIODE │
               [LOAD COIL]          └───┬───┘
              (Pump or Valve)           │
                    │                   │
                    └─────────┬─────────┘
                              │
                              ▼
                         MOSFET DRAIN
                       ┌──────────────┐
ESP32 GPIO ────[100Ω]──┤ GATE         │
(25, 26, 27)    │      │              │
                │      │    SOURCE    │
             [10kΩ]    └──────┬───────┘
             Pull-Down        │
                │             │
                ▼             ▼
               COMMON GROUND (GND)
```

---

## 4. Capacitive Soil Moisture Sensor Connections (x4 Sensors)

```
       ESP32 (30-Pin)                           Capacitive Sensor
       ┌────────────┐                            ┌──────────────┐
       │        3V3 ├────────────────────────────┤ VCC (Red)    │
       │            │                            │              │
       │        GND ├────────────────────────────┤ GND (Black)  │
       │            │                            │              │
       │    GPIO 32 ├────── Zone 1 Sensor A ─────┤ AOUT (Yellow)│
       │    GPIO 33 ├────── Zone 1 Sensor B ─────┤ AOUT (Yellow)│
       │    GPIO 34 ├────── Zone 2 Sensor A ─────┤ AOUT (Yellow)│
       │    GPIO 35 ├────── Zone 2 Sensor B ─────┤ AOUT (Yellow)│
       └────────────┘                            └──────────────┘
```

> [!CAUTION]
> **Never Connect Sensor VCC to 5V**:
> The ESP32 analog pins can only tolerate up to 3.3V. Powering the sensor with 5V will generate an analog output above 3.3V, causing permanent hardware damage to the ESP32 ADC.
