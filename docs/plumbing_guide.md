# Plumbing & Hydraulic Network Guide

This guide covers the physical assembly, flow calculations, fitting connections, and commissioning of the drip irrigation network.

---

## 1. Hydraulic Flow Architecture

```
[ Water Reservoir ]
        │
   (Flexible Hose)
        ▼
[ 120-Mesh Filter (1/2") ] ──► [ 12V Diaphragm Pump ]
                                        │
                                (1/2" High-Pressure Line)
                                        ▼
                             [ 2-Way 1/2" Manifold ]
                                    ├───► [ 12V NC Solenoid Valve 1 ] ──► 16mm LDPE (Zone 1) ──► 10x 2 LPH Emitters
                                    └───► [ 12V NC Solenoid Valve 2 ] ──► 16mm LDPE (Zone 2) ──► 10x 2 LPH Emitters
```

---

## 2. Component Installation & Piping Guidelines

### 1. Water Reservoir & 120-Mesh Filter
- **Positioning**: Install the 120-mesh (130-micron) filter **on the suction (inlet) side of the pump** or immediately after it.
- **Why**: Suspended grit, algae, and organic debris will cause the diaphragm pump valves to stick open and permanently clog the micro-orifices in the 2 LPH drip emitters.
- **Maintenance**: Unscrew the filter bowl monthly and rinse the stainless steel mesh cartridge under running water.

### 2. 12V Diaphragm Pump
- **Self-Priming**: Diaphragm pumps can draw water from a non-pressurized reservoir located below pump level (up to ~1.5m lift).
- **Mounting**: Mount horizontally on rubber vibration-damping feet.
- **Internal Pressure Switch**: Most 12V diaphragm pumps feature an integrated bypass/cutoff switch (typically 60–80 PSI). When the pump runs into closed valves, it will cycle. Our software anti-hammer sequencing guarantees a valve is always open *before* the pump runs, preventing cycling.

### 3. Solenoid Valves (12V Normally Closed)
- **Flow Direction**: Solenoid valves are **strictly directional**. Look for the embossed arrow on the valve body indicating the direction of fluid flow.
  > [!WARNING]
  > **Reverse Flow Alert**:
  > Installing a solenoid valve backwards will cause water to lift the diaphragm off its seat, resulting in continuous uncontrolled leaking even when the coil is de-energized.
- **Thread Sealing**: Wrap male threads with 4–6 layers of PTFE (Teflon) tape before screwing into the 2-way manifold. Hand-tighten plus 1 to 1.5 turns with a wrench. Do not overtighten plastic threads.

### 4. 16mm Mainline Pipe (LDPE)
- Run the 16mm low-density polyethylene pipe through your garden bed or container rows.
- Use barbed elbows for 90-degree corners to prevent kinks that choke water flow.
- Secure the end of each zone line with a 16mm figure-8 end closure or threaded flush valve.

### 5. 4mm Feeder Tubes & 2 LPH Pressure-Compensating Drippers
- Use a dedicated 3mm/4mm drip punch tool to pierce the 16mm mainline.
- Press a barbed 4mm connector into the hole, run a length of 4mm flexible microtube to the plant base, and insert the **2 LPH Pressure-Compensating (PC)** emitter.
- **Why PC Emitters**: Standard non-compensating drippers deliver more water at the beginning of the pipe and less at the far end. Pressure-compensating drippers contain a flexible silicone diaphragm that delivers exactly 2.0 LPH across pressure ranges from 0.8 bar to 3.5 bar.

---

## 3. Flow & Hydraulic Calculations

| Parameter | Zone 1 | Zone 2 | Units |
| :--- | :---: | :---: | :--- |
| **Number of Emitters** | 10 | 10 | units |
| **Discharge per Emitter** | 2.0 | 2.0 | Liters / hour (LPH) |
| **Total Zone Flow Rate** | **20.0** | **20.0** | **LPH** (~0.33 L/min) |
| **Pump Rated Output** | 300.0 | 300.0 | LPH (5.0 L/min) |
| **Hydraulic Flow Margin** | 15x excess capacity | 15x excess capacity | Guarantees full operating pressure |

Because the 12V diaphragm pump delivers 5 L/min and each zone requires only 0.33 L/min, the system easily maintains full operating pressure (20–40 PSI) across all emitters.

---

## 4. Commissioning & Line Flushing Checklist

1. **Flush Before Installing Emitters**:
   - Before punching holes and attaching drippers, leave the end of the 16mm line open.
   - Run the pump for 30 seconds via sample sketch `02_mosfet_actuator_test.ino` to blow out plastic shavings and dirt from inside the new pipe.
   - Seal the pipe ends with figure-8 clamps.
2. **Inspect for Leaks**:
   - Energize Zone 1 for 60 seconds; inspect all fittings, manifold joints, and solenoid threads.
   - Repeat for Zone 2.
3. **Verify Emitter Output**:
   - Collect output from 2 sample drippers in a measuring cup for 3 minutes.
   - Expected volume: $\approx 100\text{ mL}$ in 3 minutes ($\frac{2000\text{ mL}}{60\text{ min}} \times 3\text{ min} = 100\text{ mL}$).
