# 🔬 EXTASIS MARIMBA — Signal Architecture & DSP Engineering Document

**Document Version:** 1.0.0  
**Author:** Lauro Robles / Extasis Records  
**Target Engine:** 64-bit Floating-Point Hybrid Modal Physical Modeling & Inharmonic FM Synthesis

---

## 1. Executive Summary & Acoustic Philosophy

Traditional sampling fails to capture the true expressiveness of the **Marimba Chiapaneca** because an acoustic marimba is not a single linear resonant body, but rather a **complex coupled system of mechanical, acoustic, and aero-elastic oscillators**:

$$\text{Mallet Impact} \xrightarrow{\text{Mechanical}} \text{Wood Bar Modals} \xrightarrow{\text{Acoustic}} \text{Cavity Air Column} \xrightarrow{\text{Aero-elastic}} \text{Cachimba Membrane} \xrightarrow{\text{Radiation}} \text{Stereo Field}$$

**Extasis Marimba** implements this entire acoustic chain in pure real-time code without samples, generating zero-latency physical audio with infinite velocity gradations and natural organic micro-imperfections.

---

## 2. Global Signal Flow Diagram

```
 ┌────────────────────────────────────────────────────────────────────────────────────────┐
 │                                   VOICE SIGNAL PATH (x16)                              │
 └────────────────────────────────────────────────────────────────────────────────────────┘
                                               │
                                      [ MIDI / TRIGGER ]
                               (Note Number, Velocity, Glide)
                                               │
                        ┌──────────────────────┴──────────────────────┐
                        │                                             │
             ┌─────────────────────┐                       ┌─────────────────────┐
             │   MALLET EXCITER    │                       │  PUEBLO VARIATION   │
             │                     │                       │  (Deterministic)    │
             │ • Rubber Impulse    │                       │ • Pitch Drift (±6c) │
             │ • Noise (2.5-5.5kHz)│                       │ • Decay Drift (±18%)│
             │ • Stick Click Snap  │                       │ • Buzz Drift (±25%) │
             └──────────┬──────────┘                       └──────────┬──────────┘
                        │                                             │
                        ├─────────────────────────────────────────────┘
                        │
         ┌──────────────┴──────────────┐
         │                             │
         ▼                             ▼
┌───────────────────┐        ┌───────────────────┐
│ 5-MODE MODAL BAR  │        │   LA CACHIMBA     │
│ RESONATOR         │        │ (Membrane Buzz)   │
│                   │        │                   │
│ • Key-Track Ratios│        │ • Carrier (2*f0)  │
│   (1:3.15->1:3.60)│        │ • Modulator(7.2f0)│
│ • Wood Damping    │        │ • Vel-Reactive    │
│ • Acoustic Tube 1 │        │ • Non-Linear      │
│ • Tube Beating 2  │        │   Chatter Sat     │
│   (DOOO~WONNNG)   │        │ • 140ms Burst Env │
└─────────┬─────────┘        └─────────┬─────────┘
          │                            │
          └─────────────┬──────────────┘
                        │
                        ▼ (Summed Acoustic Voice)
             ┌─────────────────────┐
             │   SEM SVF FILTER    │
             │ 12dB Low-Pass       │
             │ Dynamic Cutoff Env  │
             │ Soft Saturation     │
             └──────────┬──────────┘
                        │
                        ▼
             ┌─────────────────────┐
             │ PERCUSSIVE ENVELOPE │
             │ Snappy Attack/Decay │
             └──────────┬──────────┘
                        │
                        ▼
             ┌─────────────────────┐
             │  STEREO PANNING     │
             │ Keyboard Key-Spread │
             │ (C1=Left, C7=Right) │
             └──────────┬──────────┘
                        │
 ┌──────────────────────┴─────────────────────────────────────────────────────────────────┐
 │                                   MASTER BUS DSP PATH                                  │
 └────────────────────────────────────────────────────────────────────────────────────────┘
                        │
                        ▼ (16-Voice Sum)
             ┌─────────────────────┐
             │   WARM SATURATION   │
             │ Analog Tube Drive   │
             └──────────┬──────────┘
                        │
                        ▼
             ┌─────────────────────┐
             │ MID/SIDE SPATIALIZER│
             │ Stereo Spread Width │
             └──────────┬──────────┘
                        │
                        ▼
             ┌─────────────────────┐
             │ ACOUSTIC AMBIENCE   │
             │ Resonator Reflections│
             └──────────┬──────────┘
                        │
                        ▼
             ┌─────────────────────┐
             │   MASTER VOLUME     │
             │ Clean Output Gain   │
             └──────────┬──────────┘
                        │
                        ├──────────────────────────► [ REAL-TIME OLED DISPLAY FIFO ]
                        ▼
                 [ STEREO OUTPUT ]
```

---

## 3. Detailed Component Breakdown

### 3.1. Mallet Exciter (`MalletExciter.h`)
The exciter generates three distinct, layered transient phenomena:
1. **Physical Mallet Impulse:** Raised cosine window scaled by hardness ($0.8\text{ ms}$ to $4.2\text{ ms}$ duration):
   $$x_{\text{impulse}}(t) = \sin\left(\frac{\pi t}{T}\right)^{1 + 2(1 - \text{hardness})}$$
2. **Rubber Mallet Noise Burst:** White noise filtered through a 2-pole Cytomic SVF bandpass centered at $2.5\text{ kHz} - 5.5\text{ kHz}$ with exponential decay ($8\text{ ms} - 24\text{ ms}$). Recreates the rubber elasticity striking tropical hardwood.
3. **Stick Click Snap:** High-frequency metallic/wood impact click for transient definition.

---

### 3.2. Modal Bar Resonator (`ModalBarResonator.h`)
Each bar is modeled as 5 parallel second-order resonant bandpass filters (Modes 1 to 5):

$$H_i(z) = \frac{g_i}{1 - 2 r_i \cos(\omega_i) z^{-1} + r_i^2 z^{-2}}$$

* **Key-Tracking Inharmonic Ratios:**
  * Fundamental Mode 1: $f_0$
  * Mode 2: Ratios scale dynamically from $3.15 \cdot f_0$ (deep bass) to $3.60 \cdot f_0$ (treble) based on physical bar arching.
  * Modes 3, 4, 5: $2.3 \times, 4.0 \times, 6.2 \times$ Mode 2.
* **Resonator Damping ($T_{60}$):**
  $$r_i = \exp\left(-\frac{6.907755}{T_{60, i} \cdot f_s}\right), \quad T_{60, i} = \frac{T_{\text{base}}}{1 + i^2 (0.7 - 0.35 \cdot \text{mat})}$$
* **Acoustic Resonator Tube with Micro-Beating (DOOO~WONNNG):**
  Two coupled acoustic cavities under the bar:
  * Tube 1: Tuned exactly to $f_0$.
  * Tube 2: Tuned to $f_0 + \Delta f$ ($\Delta f = 3.5\text{--}5.5\text{ Hz}$), generating acoustic beating and air breathing.

---

### 3.3. La Cachimba (Buzz Membrane Engine) (`BuzzMembrane.h`)
The signature sound of Chiapas marimbas is the *cachimba*—a membrane of pig intestine affixed over a wax hole at the base of the acoustic tube.
* **Carrier Frequency:** $f_c = 2 \cdot f_0$
* **Modulation Frequency:** $f_m = 7.2 \cdot f_0$
* **Dynamic Modulation Index:** Scaled by velocity squared ($Vel^2$) to simulate non-linear membrane excitation:
  $$I_{\text{FM}} = \text{Amount} \cdot \left[1.2 + 3.5 \cdot \left((1 - \text{Sens}) + \text{Sens} \cdot Vel^2\right)\right]$$
* **Asymmetric Chatter Distortion:**
  $$y_{\text{buzz}} = \begin{cases} 1.3 \cdot x_{\text{FM}} - 0.3 \cdot x_{\text{FM}}^3 & \text{if } x_{\text{FM}} > 0 \\ 0.5 \cdot x_{\text{FM}} & \text{if } x_{\text{FM}} \le 0 \end{cases}$$
* **Fast Burst Decay:** Decays in $140\text{ ms} - 260\text{ ms}$ to an ambient sustain floor, producing the punchy *«TAK–ÑAAANG–BRR»*.

---

### 3.4. "Marimba de Pueblo" (Organic Micro-Imperfections)
Artisanal instruments never exhibit identical response across every bar. Extasis Marimba uses a deterministic hash per MIDI note:
* **Pitch Drift:** $\Delta \text{pitch} \in [-6, +6]\text{ cents}$
* **Decay Drift:** $\Delta \text{decay} \in [-18\%, +18\%]$
* **Membrane Tension Drift:** $\Delta \text{buzz} \in [-25\%, +25\%]$

---

### 3.5. SEM State-Variable Filter (`SEMFilter.h`)
Cytomic-style trapezoidal integrator 12dB/octave SVF with soft input tanh saturation:
$$g = \tan\left(\frac{\pi f_c}{f_s}\right), \quad R = 2 (1 - 0.92 \cdot \text{Reso})$$
Dynamic cutoff modulated by note velocity and the primary strike envelope.

---

### 3.6. Stereo Body Processor (`StereoBodyProcessor.h`)
1. **Warm Saturation:** Non-linear analog tape/tube drive ($y = \tanh(x \cdot \text{drive})$).
2. **Keyboard Acoustic Spread:** $C_1$ (panned left) to $C_7$ (panned right) with mid/side stereophonic widening.
3. **Resonator Ambience:** Dual-delay cross-coupled reflection network ($13\text{ ms}$ & $19\text{ ms}$) simulating the acoustic wooden body and rehearsal space.

---

## 4. Threading & Concurrency Architecture

```
 ┌─────────────────────────────────────────────────────────────────────────────┐
 │                            THREAD CONCURRENCY MODEL                         │
 └─────────────────────────────────────────────────────────────────────────────┘

 [ GUI THREAD ]                                          [ AUDIO RENDER THREAD ]
  • Trigger Button Mouse Click / Drag                     • processBlock() Callback
  • Preset Box Selection                                  • APVTS Parameter Read
  • License Modal Overlay                                 • 16-Voice Polyphony DSP
       │                                                       │
       │ Atomic Lock-Free FIFO Queue (triggerFifo)             │
       ├──────────────────────────────────────────────────────►│
       │                                                       │
       │                                                       │ Audio Rendered
       │                                                       │
       │ Real-Time Audio Block Callback (onAudioBlockProcessed)│
       │◄──────────────────────────────────────────────────────┤
       │
  • Push to Display 256-sample FIFO
  • OLED Oscilloscope 30Hz Paint
  • 5 Modal Resonator Bar Heights
```

1. **Lock-Free Audition Queue:** GUI clicks on the central Logo Trigger Pad are pushed into a 16-element lock-free ring buffer (`triggerFifo`), ensuring zero audio glitching or thread contention.
2. **Real-Time Visualizer Streaming:** The audio thread dispatches non-blocking sample pointers to `MarimbaDisplay::pushAudioSamples()`, feeding a zero-crossing triggered oscilloscope and modal bar visualizer.

---

## 5. Parameter Table & CC Mapping

| Parameter ID | Display Name | Normalisable Range | Default | Description |
|---|---|---|---|---|
| `hardness` | Hardness | 0.0 – 1.0 (0.01 step) | 0.65 (65%) | Mallet rubber vs hard wood ratio |
| `noise` | Rubber Noise | 0.0 – 1.0 (0.01 step) | 0.40 (40%) | 2.5–5.5kHz mallet transient burst |
| `click` | Strike Click | 0.0 – 1.0 (0.01 step) | 0.30 (30%) | High-frequency stick click snap |
| `attack` | Attack Time | 0.1 – 50.0 ms | 0.5 ms | Initial strike enunciation |
| `decay` | Bar Decay | 0.0 – 1.0 (0.01 step) | 0.55 (55%) | Modal bar exponential ring time |
| `material` | Material | 0.0 – 1.0 (0.01 step) | 0.10 (Hormiguillo) | Hormiguillo $\to$ Balafon $\to$ Glass |
| `overtones` | Overtones | 0.0 – 1.0 (0.01 step) | 0.60 (60%) | Inharmonic modal energy balance |
| `organic` | Pueblo Drift | 0.0 – 1.0 (0.01 step) | 0.45 (45%) | Artisanal per-key micro-imperfection |
| `tube` | Tube Cavity | 0.0 – 1.0 (0.01 step) | 0.70 (70%) | Acoustic air cavity & beating depth |
| `buzz` | Cachimba Buzz| 0.0 – 1.0 (0.01 step) | 0.50 (50%) | Non-linear membrane vibration amount |
| `buzzVel` | Buzz Dynamics| 0.0 – 1.0 (0.01 step) | 0.85 (85%) | Velocity sensitivity of the buzz |
| `cutoff` | SEM Filter | 100 Hz – 20 kHz | 11.0 kHz | 12dB State-Variable Low-Pass filter |
| `spread` | Stereo Spread| 0.0 – 1.0 (0.01 step) | 0.65 (65%) | Keyboard spatial panning & width |
| `drive` | Warm Drive | 0.0 – 1.0 (0.01 step) | 0.15 (15%) | Analog tape/tube body saturation |
| `ambience` | Ambience | 0.0 – 1.0 (0.01 step) | 0.25 (25%) | Resonator body early reflections |
| `volume` | Master Vol | 0.0 – 1.5 (0.01 step) | 0.85 (85%) | Output ceiling volume |

---
*© Extasis Records — Documented & Maintained by Lauro Robles.*
