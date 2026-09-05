# 🪵 EXTASIS MARIMBA — Official User Manual
**Mexican Physical-FM Synthesizer (VST3 / AU / Standalone)**  
*Developed & Coded by Lauro Robles / Extasis Records*

---

## 1. Introduction & Acoustic Philosophy

Traditional marimbas—especially the iconic instruments of Chiapas, Oaxaca, and Guatemala—possess a sonic identity that cannot be captured by static sampling. When a rubber mallet strikes a tuned wooden bar of *Hormiguillo*, a complex chain of acoustic events unfolds in milliseconds:

$$	ext{Impact} \longrightarrow 	ext{Inharmonic Bar Resonance} \longrightarrow 	ext{Acoustic Air Tube} \longrightarrow 	ext{Vibrating Buzz Membrane (La Cachimba)} \longrightarrow 	ext{Air}$$

**Extasis Marimba** synthesizes this entire physical phenomenon in real time using a 24-bit/96kHz custom **Modal Physical Modeling & FM Synthesis Engine**.

---

## 2. Interface Overview

Extasis Marimba's interface is divided into an LCD feedback dashboard and three distinct acoustic modules. There is no complicated synth jargon; every one of the 12 knobs corresponds strictly to the physical properties of a real marimba.

### The Display (Top Left)
* **Oscilloscope:** Real-time visual representation of the acoustic waveform.
* **Modal Resonators (Meters):** React in real time to the fundamental frequency and the 4x/10x mode harmonics as they excite the virtual air tube.

### Module 1: WOODEN BAR
* **`HARDNESS`:** Controls the FM peak index and exponential decay rate. Low values simulate soft yarn mallets (warm sine wave); high values simulate hard rubber/wood mallets (spiky, bright inharmonic attack).
* **`OVERTONES`:** Dynamically sweeps energy between the 4x mode (striking the center of the bar) and the 10x mode (striking the edge of the bar).
* **`MATERIAL`:** Detunes the mathematical relationship of the modes. At 50%, the bar is perfectly tuned to 1:4:10. Turning it up or down simulates imperfect wood carving, density variations, and metallic or glass-like behaviors.
* **`CLICK / SNAP`:** Injects a micro 5-millisecond high-pass filtered transient noise (2.5kHz), reproducing the raw *"TAK"* of the stick hitting the bar.

### Module 2: RESONATOR TUBE
* **`TUBE CAVITY`:** The volume of the 1/4-wave bandpass resonator hanging below the bar. Features a 20ms acoustic delay (simulating the time sound takes to travel from the bar into the tube).
* **`BAR DECAY`:** Controls the macro-sustain of the wood and the tube (100ms - 500ms). Features aggressive key-tracking: low notes ring out indefinitely, while extreme treble notes "tock" and die instantly.
* **`MEMBRANE BUZZ`:** Non-linear physical model of the pig intestine membrane (*cachimba*) mounted with bee's wax at the bottom of the tube. Introduces the signature *«ÑAAANG–BRR»* rattle.
* **`BUZZ DYNAMICS`:** Velocity scaling threshold for the membrane. When turned up, soft hits sound like pure warm wood, while hard hits break the acoustic pressure threshold and explode with distortion.

### Module 3: ENVIRONMENT
* **`ORGANIC DRIFT`:** The "Pueblo" imperfection engine. Introduces permanent, deterministic random detuning (±8 cents) and decay variations uniquely mapped to every single MIDI note, simulating raw, weathered, mismatched wood.
* **`AMBIENCE`:** An integrated 1/4-note feedback delay simulating courtyard reflections and spatial depth.
* **`SPREAD`:** Acoustic stereo panning width. In reality, a marimba is 2 meters long. This parameter maps low notes to your physical left and high notes to your right.
* **`MASTER VOL`:** Clean master output gain (pre soft-saturation clipper).

---

## 3. Factory Preset Guide (40 Presets)

The synthesizer includes 40 categorized factory presets meticulously crafted to showcase the engine:

* **01 – 10: Traditional Mexican & Chiapaneca**
  * *E.g., 01. Chiapas Cachimba Marimba, 04. Zapateado Veracruzano, 10. Pure Rosewood Concert.*
  * Focus on heavy charleo buzz, organic drift, and classic wooden body.
* **11 – 20: Latin Club, Cumbia & Bass**
  * *E.g., 12. Cumbia Rebajada Low End, 14. Latin House Strike.*
  * Aggressive attack clicks, massive subby tube resonators, and wide synthetic spread.
* **21 – 30: Concert, Acoustic & Traditional World**
  * *E.g., 21. Clean Concert Rosewood, 25. African Balafon / Kalimba.*
  * Perfectly tuned 1:4:10 modes, soft mallets, zero buzz, and pristine classical decay.
* **31 – 40: Experimental, Glass & Hybrid FM**
  * *E.g., 31. Glass & Vibra Bars, 35. Sub-Bass Thumper Marimba.*
  * Pushing the FM boundaries into cyberpunk synths, lo-fi keys, and granular-esque destroyed wood.

---

## 4. Preset Management & User Patches
* **Saving:** Click the `SAVE` button in the top-right preset panel, enter a custom name, and press Return.
* **Storage Location:** User presets are saved as XML `.empreset` files in:
  * **macOS:** `~/Documents/ExtasisRecords/ExtasisMarimba/Presets/`
  * **Windows:** `%USERPROFILE%\Documents\ExtasisRecords\ExtasisMarimba\Presets\`
* **Loading:** All user presets are automatically enumerated in the dropdown under `[User] <Preset Name>`.

---

## 5. Licensing & Registration
Extasis Marimba includes a **10-minute full-featured evaluation period**. To unlock permanently:
1. Click the **`[ DEMO ]`** badge in the upper right header.
2. Enter your 16-character serial key (format: `EXTM-XXXX-XXXX-XXXX-XXXX`).
3. Click **ACTIVATE LICENSE**. The badge will change to **`[ LICENSED ]`** and the key is securely saved offline to your local application support directory.

> **Get your official license at:** [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)

---
*© Extasis Records — Built with JUCE 8 & Advanced Physical-FM DSP.*
