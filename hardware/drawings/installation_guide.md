# Field Installation, Weatherproofing & Physical Assembly Guide

This guide details physical enclosure layout, sensor weatherproofing, soil insertion depth, and field deployment techniques.

---

## 1. Electronics Enclosure (IP65 Weatherproof Box)

For outdoor or greenhouse installation, mount all electronics inside an **IP65-rated polycarbonate enclosure**:

```
┌────────────────────────────────────────────────────────┐
│                   IP65 ENCLOSURE                       │
│                                                        │
│  ┌──────────────────┐            ┌──────────────────┐  │
│  │ DC-DC BUCK       │            │ 3x MOSFET MODULES│  │
│  │ CONVERTER        │            │ (Pump, V1, V2)   │  │
│  │ (12V -> 5V)      │            │ [1N4007 Diodes]  │  │
│  └────────┬─────────┘            └────────▲─────────┘  │
│           │                               │            │
│           ▼ 5.0V Rail                     │ GPIO Lines │
│  ┌────────────────────────────────────────┴─────────┐  │
│  │            ESP32 DEVKIT V1 (30-PIN)              │  │
│  │         (Mounted on Terminal Breakout)           │  │
│  └────────────────────┬─────────────────────────────┘  │
│                       │ 3.3V + ADC Lines               │
│                       ▼                                │
│   [ Cable Gland 1 ]  [ Cable Gland 2 ]  [ Cable Gland 3]│
└─────────┬──────────────────┬──────────────────┬────────┘
          │                  │                  │
          ▼                  ▼                  ▼
      12V 5A In          Actuators          4x Sensors
     (DC Jack)       (Pump & Solenoids)     (Shielded)
```

### Installation Best Practices
- **Cable Glands**: Use **PG7 or PG9 nylon cable glands** on the bottom of the enclosure to prevent water ingress along the cables.
- **Drip Loop**: Always create a U-shaped "drip loop" in every incoming wire before it enters the cable gland so rainwater drips off rather than tracking into the box.
- **Silica Gel**: Place a 10g packet of rechargeable silica gel inside the enclosure to absorb ambient humidity.

---

## 2. Sensor Weatherproofing (Crucial for Capacitive Sensors)

Although the white probe blade of capacitive soil moisture sensors is waterproof, the **top rectangular section contains exposed surface-mount electronics** (the 555-timer IC, resistors, capacitors, and connector pins). 

> [!WARNING]
> **Corrosion Risk on Sensor Head**:
> If rainwater, condensation, or fertilizer spray touches the top electronic section, the sensor will corrode, short-circuit, or deliver erratic readings.

### How to Waterproof the Sensor Head
1. **Remove Plastic Jumper Socket**: Solder wires directly to the 3 sensor solder pads (`VCC`, `GND`, `AOUT`) for a permanent, vibration-proof connection.
2. **Apply Conformal Coating**: Paint the top electronic circuit with two coats of clear polyurethane conformal coating or transparent silicone sealant.
3. **Adhesive Heat-Shrink**: Slip a piece of **dual-wall heat-shrink tubing with heat-activated adhesive** over the top 3 cm of the sensor (covering the circuitry and the wire solder joints).
4. **Heat & Seal**: Heat with a heat gun until the internal hot-melt adhesive oozes from both ends, forming an impermeable, hermetic seal.

---

## 3. Soil Sensor Placement & Insertion Depth

1. **Depth**:
   - **Shallow Sensor (Sensor A)**: Insert vertically at a depth of **5 to 8 cm** (monitors surface evaporation and top root zone).
   - **Deep Sensor (Sensor B)**: Insert at a depth of **12 to 18 cm** (monitors deep root reserve moisture).
2. **Firm Contact**:
   - Dig a clean slit with a trowel, insert the sensor blade firmly, and pack the soil firmly around the blade.
   - Air gaps between the sensor blade and soil will cause artificially dry readings.
3. **Distance from Drip Emitters**:
   - Place sensors **8 to 12 cm away** from the nearest drip emitter.
   - Do not position the sensor directly beneath an emitter (which creates a local saturated spot not representative of the root zone).

---

## 4. Mechanical Mounting & Pump Vibration Damping

1. **Pump Vibration Isolation**:
   - Diaphragm pumps produce mechanical pulses. Mount the pump using its flexible rubber mounting grommets to a rigid backing board.
   - Use flexible reinforced silicone or braided PVC hose for the first 30 cm of both inlet and outlet before transitioning to rigid pipe to absorb vibrations.
2. **Solenoid Manifold Support**:
   - Secure the 2-way manifold with pipe clips to prevent the weight of the solenoid valves and water pressure from flexing the fittings.
