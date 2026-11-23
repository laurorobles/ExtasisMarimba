# 🔬 EXTASIS MARIMBA — Signal Architecture & DSP Engineering Document

**Document Version:** 4.0.0 (Free & Open Source — GPLv3)  
**Author:** Lauro Robles / Extasis Records  
**Engine:** Lock-Free C++17 Physical-FM 1:4:10 Tuned Bar & Acoustic Resonator Engine

---

## 1. Executive Summary & Acoustic Philosophy

Traditional sampling fails to capture the true expressiveness of the **Marimba Chiapaneca** because an acoustic marimba is a **coupled system of mechanical, acoustic, and aero-elastic oscillators**:

$$\text{Mallet Impact} \xrightarrow{\text{FM 1:4:10}} \text{Wooden Bar} \xrightarrow{\text{20 ms Delay}} \text{1/4-Wave Tube Cavity} \xrightarrow{\text{Pressure Gate}} \text{La Cachimba Buzz} \xrightarrow{\text{Equal-Power}} \text{Stereo Room}$$

**Extasis Marimba** synthesizes this entire acoustic chain in real time with zero samples, zero mutexes on the audio thread, and zero heap allocations during rendering.

---

## 2. Global Signal Flow Diagram

```
 ┌────────────────────────────────────────────────────────────────────────────────┐
 │                            VOICE SIGNAL PATH (x16)                             │
 └────────────────────────────────────────────────────────────────────────────────┘
                                        │
                    [ MIDI Note / Velocity / Sustain CC64 / Pitch Bend ]
                                        │
                  ┌─────────────────────┴─────────────────────┐
                  │                                           │
       ┌──────────────────────┐                    ┌──────────────────────┐
       │   1:4:10 FM CORE     │                    │   PUEBLO DRIFT PRNG  │
       │ • Carrier + PitchEnv │                    │ • Per-Key Detune ±8c │
       │ • ModA (4.0x Mode)   │                    │ • Per-Key Decay Var  │
       │ • ModB (10.0x Mode)  │                    │ • Per-Strike XorShift│
       │ • Nyquist-Safe Guard │                    └──────────────────────┘
       └──────────┬───────────┘
                  │  + 2.5 kHz HPF Click Noise (5 ms)
                  ▼ (Bar Source)
       ┌──────────────────────┐
       │  20ms ACOUSTIC DELAY │──────► Direct Bar Signal
       │  + BPF TUBE (Q=15)   │──────► Resonator Tube Bloom
       └──────────┬───────────┘
                  │
                  ▼
       ┌──────────────────────┐
       │ LA CACHIMBA MEMBRANE │  (Pressure-Gated White Noise × Tube Signal)
       └──────────┬───────────┘
                  │
                  ▼
       ┌──────────────────────┐
       │ SOFT SATURATION tanh │  + Equal-Power Key-Tracked Stereo Panning
       └──────────┬───────────┘
                  │
 ┌────────────────┴───────────────────────────────────────────────────────────────┐
 │                              MASTER BUS DSP PATH                               │
 └────────────────────────────────────────────────────────────────────────────────┘
                  │
                  ▼ (16-Voice Stereo Sum)
       ┌──────────────────────┐
       │  STEREO AMBIENCE     │  Decorrelated Courtyard Delays (L:15/33ms, R:19/41ms)
       └──────────┬───────────┘
                  │
                  ▼
       ┌──────────────────────┐
       │ SMOOTHED MASTER VOL  │  (Per-sample exponential smoothing)
       └──────────┬───────────┘
                  │
                  ├──────────────────────► [ LOCK-FREE SPSC RING BUFFER (2048) ]
                  ▼
           [ STEREO OUTPUT ]
```

---

## 3. Threading & Real-Time Safety Architecture

1. **Lock-Free Audition Queue (`triggerFifo`):** GUI Trigger Pad clicks and pitch drags push into a 32-slot atomic SPSC FIFO (`memory_order_acquire` / `release`).
2. **Lock-Free Visualizer Telemetry (`scopeRingBuffer` & `outputRmsAtomic`):** The audio thread writes mono-summed samples into a fixed-size 2048-sample ring buffer and stores RMS/voice counts atomically—zero mutexes, zero allocations. `MarimbaDisplay` pulls from this buffer on its 30 Hz UI timer.
3. **Per-Voice Lock-Free PRNG (`FastRng`):** Every voice owns an independent `XorShift32` generator, eliminating all `rand()` / global RNG locks on the audio thread.
4. **Smart Voice Stealing:** When all 16 voices are active, `findFreeVoice()` selects the quietest decaying voice (`lowest ampEnv.getCurrent()`) to minimize audible artifacts.

---

## 4. Parameter Table (12 APVTS Parameters)

| Parameter ID | Display Name | Range | Default | Description |
|---|---|---|---|---|
| `hardness` | Hardness | 0.0 – 1.0 | 0.65 | FM peak modulation index & modal decay rate |
| `overtones` | Overtones | 0.0 – 1.0 | 0.40 | Balance between 4× (center) and 10× (edge) modes |
| `material` | Material | 0.0 – 1.0 | 0.50 | Mode ratio carving detune (0.50 = exact 1:4:10) |
| `click` | Click / Snap | 0.0 – 1.0 | 0.35 | 2.5 kHz HPF mallet contact transient level |
| `tube` | Tube Cavity | 0.0 – 1.0 | 0.85 | 1/4-wave bandpass resonator cavity volume |
| `decay` | Bar Decay | 0.0 – 1.0 | 0.55 | Key-tracked macro amplitude decay (100–550 ms) |
| `buzz` | Membrane Buzz | 0.0 – 1.0 | 0.60 | *La Cachimba* membrane rattle intensity |
| `buzzVel` | Buzz Dynamics | 0.0 – 1.0 | 0.65 | Velocity threshold sensitivity for buzz gate |
| `organic` | Organic Drift | 0.0 – 1.0 | 0.45 | Per-key detuning (±8c) & per-strike variation |
| `ambience` | Ambience | 0.0 – 1.0 | 0.25 | Decorrelated stereo courtyard early reflections |
| `spread` | Spread | 0.0 – 1.0 | 0.60 | Equal-power key-tracked stereo panning width |
| `volume` | Master Vol | 0.0 – 1.0 | 0.85 | Smoothed master output gain |

---
*© 2026 Lauro Robles / Extasis Records — Licensed under GNU GPLv3.*
