# 🪵 Extasis Marimba (v1.0)

<p align="center">
  <strong>Physical Modeling & Modal Resonator Synthesizer Workstation</strong><br>
  <em>Built with JUCE 7 (C++17/C++20) for macOS (Universal Binary: Apple Silicon & Intel) & Windows (x64).</em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Version-1.0.0-blue.svg" alt="Version 1.0" />
  <img src="https://img.shields.io/badge/Platform-macOS%20%7C%20Windows-lightgrey.svg" alt="Platform" />
  <img src="https://img.shields.io/badge/Format-VST3%20%7C%20AU%20%7C%20Standalone-orange.svg" alt="Format" />
  <img src="https://img.shields.io/badge/Engine-Modal%20Physical%20Modeling-green.svg" alt="Engine" />
  <img src="https://img.shields.io/badge/License-Commercial%20%2F%20Extasis-red.svg" alt="License" />
</p>

---

## ✨ Overview

**Extasis Marimba** is a dedicated virtual instrument based on acoustic physical modeling and the acclaimed modal synthesis engine of the **Arturia MicroFreak** (`MARIMBITA` preset architecture).

It combines real-time physical mallet strike excitation, a 5-band tuned wooden bar resonator bank ($f_0$, $4 f_0$, $9.2 f_0$, $16.0 f_0$, and acoustic pipe cavity), continuous material morphing (Wood $\rightarrow$ Balafon $\rightarrow$ Glass/Vibraphone), a 12dB SEM state-variable filter, percussive snap envelopes, and spatial stereo wood diffusion.

---

## 🌟 Key Highlights

* **Physical Mallet Strike Exciter:**
  * Configurable mallet hardness (felt to hard rubber/wood) and transient click definition.
  * Natural dynamic response to MIDI velocity with timbre and hardness scaling.
* **5-Mode Modal Bar Resonator:**
  * Tuned double-octave overtones ($4 \times f_0$) matching professional concert marimbas.
  * Frequency-dependent wooden bar damping with realistic high-frequency energy decay.
  * **Material Knob:** Seamless morphing from warm Honduras Rosewood to African Balafon and sparkling metallic/glass bars.
  * **Pipe Body Resonator:** Acoustic cavity simulation tuned to fundamental pitch ($f_0$) providing rich acoustic low-end projection.
* **12dB SEM State-Variable Filter (VCF):**
  * Analog-modeled filter with soft saturation, dynamic cutoff modulation, and resonance.
* **Percussive Snap Envelope & Strike Dynamic Modulation:**
  * Ultra-snappy attack curve for punchy transients in busy mixes.
* **Stereo Wood Spread & Warmth:**
  * Key-position acoustic panning, warm saturation drive, and acoustic room reflections.
* **Interactive OLED / LCD Display:**
  * Real-time harmonic modal spectrum visualizer and mallet strike transient oscilloscope.
* **5 Calibrated Built-in Presets:**
  * `01: MicroFreak Marimbita (Default)`
  * `02: Concert Wooden Marimba`
  * `03: African Balafon / Kalimba`
  * `04: Glass & Metal Bar`
  * `05: Plucked Mallet Synth`

---

## 📖 Documentation

* 📘 **[Manual de Usuario Completo (MANUAL.md)](MANUAL.md)**
* 📋 **[Ficha Técnica de Especificaciones (FICHA_TECNICA.md)](FICHA_TECNICA.md)**

---

## 🛠️ Building from Source

### Prerequisites:
* **CMake 3.15+**
* **C++17 / C++20 Compiler:** Apple Clang (macOS) / Visual Studio 2022 MSVC (Windows) / GCC (Linux)
* **JUCE 7.x** (included or pointed via CMake)

### Build Steps:

```bash
# 1. Clone the repository
git clone --recursive https://github.com/laurorobles/ExtasisMarimba.git
cd ExtasisMarimba

# 2. Configure build with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Compile VST3, AU, and Standalone targets
cmake --build build --config Release --parallel
```

### Installation Paths:

* **macOS VST3:** `~/Library/Audio/Plug-Ins/VST3/ExtasisMarimba.vst3`
* **macOS AU (Component):** `~/Library/Audio/Plug-Ins/Components/ExtasisMarimba.component`
* **macOS Standalone:** `build/ExtasisMarimba_artefacts/Release/Standalone/ExtasisMarimba.app`

---

## 🔑 Offline Cryptographic Licensing

Extasis Marimba includes an offline cryptographic serial verification system:

```bash
# Generate a single serial key
python3 generate_license.py --single

# Generate a customer gift key
python3 generate_license.py --gift "user@email.com"

# Batch export keys for Plugin Boutique / Gumroad
python3 generate_license.py --batch 500 --output serials.txt

# Verify a key
python3 generate_license.py --verify EXTM-XXXX-XXXX-XXXX-XXXX
```

---

## 📄 License & Credits

* **Developed by:** Extasis Records / Lauro Robles
* **Based on:** Arturia MicroFreak Physical Modeling & Modal Resonator Architecture
* **Framework:** JUCE Framework
