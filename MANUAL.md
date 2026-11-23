# 🪵 EXTASIS MARIMBA — Official User Manual
**Free & Open-Source Mexican Physical-FM Synthesizer (VST3 / AU / Standalone)**  
*Developed & Coded by Lauro Robles / Extasis Records — Licensed under GPLv3*

---

## 1. Introduction & Acoustic Philosophy

Traditional marimbas—especially the iconic instruments of Chiapas, Oaxaca, and Guatemala—possess a sonic identity that cannot be captured by static sampling. When a rubber mallet strikes a tuned wooden bar of *Hormiguillo*, a complex chain of acoustic events unfolds in milliseconds:

$$\text{Impact} \longrightarrow \text{1:4:10 Bar Resonance} \longrightarrow \text{20ms Air Tube} \longrightarrow \text{Membrane Buzz (La Cachimba)} \longrightarrow \text{Stereo Room}$$

**Extasis Marimba** synthesizes this entire physical phenomenon in real time using a custom, lock-free C++ **Physical-FM & Acoustic Resonator Engine**.

---

## 2. Interface Overview

Extasis Marimba's interface (`680 × 420 px`) features a header bar with quick-action tools, a real-time LCD dashboard, a preset browser & logo trigger pad, and three symmetrical acoustic modules. **Double-click any knob** to reset it to its default value.

### Header Quick Actions
* **`INIT`:** Resets all 12 controls to a clean, balanced 1:4:10 concert rosewood marimba.
* **`RND`:** Generates a musically constrained randomized patch (preserving your Master Volume for ear safety).
* **`GPLv3 // FREE`:** Direct link to the open-source repository on GitHub.

### The LCD Display (Top Left)
* **Oscilloscope:** Real-time, zero-crossing stabilized output waveform driven by a lock-free audio ring buffer.
* **Live Voice Counter:** Displays active polyphony (`VOICES X/16`).
* **1:4:10 Modes & Cavity Meters:** Five dynamic bars (`f0`, `4f`, `10f`, `TUBE`, `BUZZ`) reacting in real time to output energy and your current sound design parameters.

### Preset Bar & Logo Trigger Pad (Top Right)
* **Preset Browser (`<`, `>`, `SAVE`):** Browse all 40 factory presets and custom user patches, or click `SAVE` to store your own `.empreset` file.
* **Extasis Logo Trigger Pad:** Click to audition the current patch; drag vertically while holding to shift the audition pitch by ±24 semitones (live note shown in the bottom-left badge, e.g., `PAD: C4`).

---

### Module 1: WOODEN BAR
* **`HARDNESS`:** Controls the FM peak index and transient decay rate. Low values simulate soft yarn mallets (warm fundamental); high values simulate hard rubber/wood mallets (bright, percussive attack).
* **`OVERTONES`:** Sweeps energy between the 4× mode (center bar strike) and the 10× mode (edge bar strike).
* **`MATERIAL`:** Adjusts mode ratio carving. At 50%, the bar is tuned to 1:4:10 *Hormiguillo* wood. Lower values yield darker, softer wood; higher values push toward glassy/metallic inharmonicity.
* **`CLICK / SNAP`:** Injects a 5ms high-pass filtered noise burst (2.5 kHz), reproducing the raw *"TAK"* of the mallet tip contacting the bar.

### Module 2: RESONATOR TUBE
* **`TUBE CAVITY`:** Level of the 1/4-wave narrow bandpass resonator (Q = 15.0) hanging below each bar, coupled via a 20ms acoustic distance delay.
* **`BAR DECAY`:** Macro-decay time (100 ms – 550 ms base) with strong physical key-tracking: bass notes ring out richly, while treble notes snap and decay quickly.
* **`MEMBRANE BUZZ`:** Non-linear physical model of the pig-intestine membrane (*La Cachimba*) mounted with beeswax at the bottom of the tube, producing the signature *«ÑAAANG–BRR»* rattle.
* **`BUZZ DYNAMICS`:** Velocity pressure threshold for the membrane. Higher values keep soft notes pure and warm while unleashing buzz only on loud accents.

### Module 3: ENVIRONMENT
* **`ORGANIC DRIFT`:** The *"Marimba de Pueblo"* imperfection engine. Applies deterministic per-key detuning (±8 cents), resonator variance, and decay shifts plus subtle per-strike randomization.
* **`AMBIENCE`:** Decorrelated stereo courtyard/floor early reflections with asymmetric L/R tap times (15/33 ms left, 19/41 ms right) and subtle cross-feedback.
* **`SPREAD`:** Equal-power acoustic stereo panning width—maps low bars to the left and high bars to the right.
* **`MASTER VOL`:** Smoothed master output gain.

---

## 3. MIDI & Expressiveness
* **Polyphony:** 16 voices with intelligent quietest-voice stealing.
* **Sustain Pedal (CC 64):** Full damper/sustain pedal support.
* **Pitch Bend:** Smooth ±2 semitone pitch wheel response.

---

## 4. Factory Preset Guide (40 Presets)
* **01 – 10: Traditional Mexican & Chiapaneca** — Authentic *charleo* buzz, *hormiguillo* resonance, and *Marimba de Pueblo* drift.
* **11 – 20: Latin Club, Cumbia & Bass** — Punchy attack clicks, deep sub-resonators, and wide club stereo spread.
* **21 – 30: Concert, Acoustic & Traditional World** — Pure 1:4:10 concert rosewood, soft yarn mallets, balafon, gamelan, and bamboo.
* **31 – 40: Experimental, Glass & Hybrid FM** — Vibraphone/glass bars, ambient dream chimes, cyberpunk leads, and lo-fi percussion.

---

## 5. Preset Management & User Patches
* **Saving:** Click `SAVE` in the top-right preset bar, type a name, and press Return.
* **Storage Location:** User presets are saved as XML `.empreset` files in:
  * **macOS:** `~/Documents/ExtasisRecords/ExtasisMarimba/Presets/`
  * **Windows:** `%USERPROFILE%\Documents\ExtasisRecords\ExtasisMarimba\Presets\`
  * **Linux:** `~/Documents/ExtasisRecords/ExtasisMarimba/Presets/`
* **Loading:** User presets automatically appear in the dropdown menu prefixed with `[User]`.

---

## 6. Open Source License
Extasis Marimba is **100% Free & Open Source** under the **GNU General Public License v3.0 (GPLv3)**. No serial keys, no activation, no time limits. Source code is available at [github.com/laurorobles/ExtasisMarimba](https://github.com/laurorobles/ExtasisMarimba).

---
*© 2026 Lauro Robles / Extasis Records — Built with JUCE 8 & Physical-FM DSP.*
