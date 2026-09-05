# 📋 TECHNICAL SPECIFICATIONS — EXTASIS MARIMBA (v3.0)

### 1. GENERAL INFORMATION
* **Product Name:** Extasis Marimba
* **Version:** 3.0.0 (Release)
* **Developer:** Extasis Records / Lauro Robles
* **Type:** Physical-FM Percussion Synthesizer
* **Inspiration:** Traditional Mesoamerican Acoustic Marimba Physics
* **Language:** C++20
* **Framework:** JUCE Framework 8.x

### 2. AUDIO & DSP SPECIFICATIONS
* **Polyphony:** 16 voices with intelligent voice stealing algorithm.
* **Sample Rates:** 44.1 kHz up to 192 kHz support.
* **Internal Processing:** 32-bit / 64-bit Floating Point with denormal prevention.
* **Core Engine:** Custom Inharmonic FM Synthesizer avoiding static wavetables or samples.
* **Mode Tuning Ratios:** Precise 1:4:10 mode implementation mapping physical bar carving.
* **Acoustic Delay:** Phase-accurate 20ms micro-delay between exciter and tube resonator.

### 3. DSP PARAMETERS (12 KNOBS)
* `Hardness`: FM peak index & decay rate map (0.0 = Soft sine, 1.0 = Spiky transient)
* `Overtones`: Dynamic mix between 4f (center strike) and 10f (edge strike) modes
* `Material`: Micro-detuning of standard mode ratios to simulate wood density
* `Click/Snap`: 2.5kHz High-Pass filtered noise burst (5ms envelope) level
* `Tube Cavity`: Volume of the 1/4-wave bandpass resonator (Q=15.0)
* `Bar Decay`: Global amplitude macro-decay (100ms - 500ms) with high-note keytracking
* `Membrane Buzz`: Amplitude of the pressure-gated Cachimba distortion
* `Buzz Dynamics`: Velocity threshold required to trigger the membrane buzz
* `Organic Drift`: Seeded `std::mt19937` randomization of pitch (±8 cents) and decay per key
* `Ambience`: Integrated 1/4-note feedback delay simulating plaza/room reflections
* `Spread`: Acoustic key-tracked panning width (Low notes = left, High notes = right)
* `Volume`: Master output level pre-saturation

### 4. SYSTEM REQUIREMENTS
* **macOS:** 10.15 Catalina or higher (Native Apple Silicon M1/M2/M3 & Intel).
* **Windows:** Windows 10 / 11 (64-bit).
* **Formats:** VST3, AU, Standalone App.

> **Licenses:** Get your official license at [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)
