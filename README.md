# 🪵 EXTASIS MARIMBA
### Mexican Physical-FM Synthesizer (VST3 / AU / Standalone)
**Developed & Coded by [@laurorobles](https://github.com/laurorobles) // [Extasis Records](https://extasisrecords.bandcamp.com)**

[![License: Proprietary](https://img.shields.io/badge/License-Proprietary-orange.svg)](https://extasisrecords.bandcamp.com)
[![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Windows-blue.svg)](https://github.com/laurorobles)
[![Format](https://img.shields.io/badge/Format-VST3%20%7C%20AU%20%7C%20Standalone-green.svg)](https://github.com/laurorobles)
[![Architecture](https://img.shields.io/badge/DSP-Physical--FM%20Modal-red.svg)](https://extasisrecords.bandcamp.com)

<p align="center">
  <img src="assets/screenshot.png" alt="Extasis Marimba Interface" width="900" style="border-radius: 8px; box-shadow: 0 4px 16px rgba(0,0,0,0.4);" />
</p>
---

## 🌟 Overview

**Extasis Marimba** is an incredibly detailed, CPU-efficient virtual synthesizer designed to capture the exact acoustic physics, timbral nuance, and unmistakable percussive soul of the **traditional Mexican marimba (Marimba Chiapaneca / Oaxaqueña)**. 

Unlike heavy sample libraries that rely on static velocity layers, **Extasis Marimba** synthesizes every strike in real-time from the ground up using a custom C++ **Modal Physical Modeling & Inharmonic FM Engine**. By avoiding sampling, the instrument is incredibly lightweight (sub-10MB) while offering a living, breathing acoustic behavior that responds organically to every MIDI input.

---

## ✨ Comprehensive Feature List

### 🪵 1. The Wooden Bar (Tuned FM Mode Synthesis)
* **1:4:10 Tuned Mode Mathematics:** Standard uniform wood bars vibrate at messy, dissonant ratios (1:2.76:5.4). Real Mexican marimbas are hand-carved underneath to tune these upper modes perfectly to a 1:4:10 relationship (Fundamental, 2 Octaves, and 3 Octaves + Major Third). Our FM core perfectly mimics this artisanal carving.
* **Dynamic Exponential Velocity (Velocity Mapping):** Tapping lightly produces a pure, warm fundamental tone (almost a sine wave). Hitting hard unleashes chaotic FM energy, inharmonic bite, and a harsh transient, perfectly mirroring how real wood responds to physical force.
* **Stick Transient (Click / Snap):** A dedicated 5-millisecond high-pass filtered noise generator (at 2.5kHz) accurately reproduces the *“TAK”* of raw rubber/wooden mallets hitting the bar before the pitch even develops.

### 🏺 2. The Resonator & Acoustic Interaction
* **Acoustic Delay:** In physical reality, sound takes time to travel from the wooden bar down into the wooden resonator tube. Extasis Marimba models this with a 20ms micro-delay between the bar attack and the tube bloom (*“TAK → TANG → BOONG”*).
* **1/4 Wave Tube Cavity:** A highly resonant bandpass filter that strictly key-tracks to the fundamental frequency, recreating the deep, hollow resonance of the wooden *cajón* hanging underneath each key.
* **Aggressive Key-Tracked Decay:** Physical physics dictate that massive bass bars ring out for seconds, while tiny treble bars die instantly. Our DSP strictly enforces this curve across the MIDI range for breathtaking realism.

### 🐝 3. La Cachimba (The Membrane Buzz)
* **Dynamic Charleo Engine:** The true soul of the Mexican marimba. We modeled the pig-intestine membrane (*tela de tripa de cerdo*) affixed with bee's wax at the base of the resonator. 
* **Pressure-Threshold Gating:** The *Charleo* only buzzes when the acoustic pressure inside the tube breaks a specific velocity threshold, creating that signature *«ÑAAANG–BRR»* rattle exactly when it should.

### 🏘️ 4. The Environment (Artisanal Imperfection)
* **Organic "Pueblo" Drift:** We implemented a seeded pseudo-random number generator (`std::mt19937`) to ensure every single key is unique. The *Organic Drift* knob introduces permanent, unique detuning (±8 cents) and decay variations for every single MIDI note, simulating raw, weathered, mismatched wood.
* **Acoustic Panning (Key-Tracked Stereo):** A marimba is a 2-meter long instrument. Low notes are synthesized hard left, and high notes hard right. The *Spread* knob lets you widen or narrow this 3D physical field.
* **Soft Saturation:** The master output runs through a mathematical `std::tanh` soft-clipper, applying 1-3dB of gentle "analog" compression that glues the transient snap and the resonant tube together.

### 🎛️ 5. Elegant User Experience
* **12 Abstract Physical Controls:** No confusing synth jargon. The UI uses 12 highly musical parameters: *Hardness, Overtones, Material, Click/Snap, Tube Cavity, Bar Decay, Membrane Buzz, Buzz Dynamics, Organic Drift, Ambience, Spread, and Master Vol*.
* **Custom LCD Dashboard:** Featuring a real-time reactive oscilloscope and modal overtone meters for immediate visual feedback.
* **Elegant 660x410 UI:** Beautiful dark brushed obsidian metal aesthetic with Hormiguillo Rosewood accents and comfortable spacing.
* **40 Categorized Factory Presets:** 
  * *Traditional Mexican & Chiapaneca (01 - 10)*
  * *Latin Club, Cumbia & Bass (11 - 20)*
  * *Concert, Acoustic & Traditional World (21 - 30)*
  * *Experimental, Glass & Hybrid FM (31 - 40)*
* **Elegant Offline Licensing:** A completely unobtrusive, sleek activation overlay to enter your serial key via Gumroad. 

---

## 🚀 Quick Installation

### macOS (Apple Silicon & Intel)
1. Download the latest macOS release archive.
2. Run `install_mac.sh` or manually copy:
   * **VST3:** `~/Library/Audio/Plug-Ins/VST3/`
   * **AU:** `~/Library/Audio/Plug-Ins/Components/`
   * **Standalone:** `/Applications/`

### Windows (64-bit)
1. Download the Windows release archive.
2. Run `install_windows.bat` or copy `ExtasisMarimba.vst3` to `C:\Program Files\Common Files\VST3\`.

---

## 🛠️ Building from Source

### Prerequisites
* **CMake** (>= 3.22)
* **C++20** compatible compiler (Clang / AppleClang / GCC / MSVC)
* **JUCE 8** (Embedded or system installed)

### Build Commands
```bash
# Clone the repository
git clone https://github.com/laurorobles/ExtasisMarimba.git
cd ExtasisMarimba

# Configure and Build Release
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

---

## 🔒 License & Registration
Extasis Marimba operates with a **10-minute full-featured evaluation period**. Full licenses can be securely registered offline via a 16-character serial key from the sleek in-app activation overlay.

> **Get your official license at:** [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)

---
*© Extasis Records — Made with ❤️ in Mexico.*
