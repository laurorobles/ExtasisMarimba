# 📋 TECHNICAL SPECIFICATIONS — EXTASIS MARIMBA (v4.0)

### 1. GENERAL INFORMATION
* **Product Name:** Extasis Marimba
* **Version:** 4.0.0 (Free & Open Source Release)
* **License:** GNU General Public License v3.0 (GPLv3)
* **Developer:** Lauro Robles / Extasis Records
* **Type:** Physical-FM Percussion Synthesizer
* **Language:** C++17
* **Framework:** JUCE Framework 8.x

### 2. AUDIO & DSP SPECIFICATIONS
* **Polyphony:** 16 voices with quietest-voice stealing algorithm.
* **Sample Rates:** 44.1 kHz to 192 kHz (dynamic delay buffer allocation & Nyquist filter/modulator guarding).
* **Real-Time Safety:** 100% lock-free audio thread — per-voice `XorShift32` PRNG, zero heap allocations, lock-free SPSC scope ring buffer (`2048` samples) & audition note FIFO.
* **Core Engine:** Custom Inharmonic 2-Modulator FM Synthesizer with 1:4:10 tuned bar ratios (Fundamental, 4.0×, 10.0×) and transient pitch envelope (+10 cents).
* **Acoustic Resonator:** 20ms acoustic distance delay feeding a key-tracked 2nd-order Biquad Bandpass Filter (Q = 15.0).
* **Spatialization:** Equal-power key-tracked stereo panning (`cos/sin`) + decorrelated stereo courtyard/floor early reflections (L: 15/33 ms, R: 19/41 ms).
* **MIDI Support:** Velocity-sensitive Note On/Off, Sustain Pedal (CC 64), Pitch Bend (±2 semitones), All Notes Off.

### 3. DSP PARAMETERS (12 KNOBS)
* `hardness`: FM peak index & modal envelope decay rate (0.0 = soft yarn, 1.0 = hard rubber/wood)
* `overtones`: Spectral balance between 4× mode (center strike) and 10× mode (edge strike)
* `material`: Mode ratio carving detune around 1:4:10 (0.50 = exact 1:4:10 *Hormiguillo*)
* `click`: 2.5 kHz High-Pass filtered mallet impact noise burst (5 ms exponential envelope)
* `tube`: 1/4-wave bandpass resonator cavity bloom level
* `decay`: Macro amplitude decay (100 ms – 550 ms) with strong key-tracking curve
* `buzz`: Pressure-gated *La Cachimba* membrane rattle level
* `buzzVel`: Velocity threshold required to engage membrane buzz
* `organic`: Deterministic per-key detuning (±8 cents), tube/decay variance, and per-strike randomization
* `ambience`: Stereo decorrelated early reflection room mix
* `spread`: Equal-power acoustic key-tracked stereo panning width
* `volume`: Smoothed master output gain

### 4. SYSTEM REQUIREMENTS
* **macOS:** 10.15 Catalina or higher (Universal Binary: Apple Silicon & Intel) — VST3, AU, Standalone.
* **Windows:** Windows 10 / 11 (64-bit) — VST3, Standalone.
* **Linux:** Ubuntu 22.04+ / modern x64 distros — VST3, Standalone.

### 5. SOURCE CODE
* Repository: [https://github.com/laurorobles/ExtasisMarimba](https://github.com/laurorobles/ExtasisMarimba)
