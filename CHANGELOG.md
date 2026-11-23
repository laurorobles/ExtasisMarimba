# 📜 Changelog — Extasis Marimba

## [4.0.0] - 2026-11-23 (Free & Open-Source Milestone Release)
### Open Source & Licensing
* **100% Free & Open Source (GPLv3):** Removed all DRM, offline serial activation, activation overlay, and 10-minute demo time restrictions. Added GNU GPLv3 `LICENSE`.
* **Repository Cleanup:** Removed unused legacy DSP/GUI headers and license generation scripts for a clean, contributor-friendly C++17 codebase.

### DSP & Audio Engine Improvements
* **100% Lock-Free Real-Time Audio Thread:** Replaced `rand()` and `juce::Random` with fast per-voice `XorShift32` PRNGs, and replaced the mutex-locked oscilloscope callback with a zero-allocation lock-free SPSC ring buffer (`2048` samples).
* **Equal-Power Key-Tracked Panning:** Upgraded linear panning to proper equal-power (`cos`/`sin`) stereo panning.
* **Decorrelated Stereo Ambience:** Replaced mono-summed floor delay with dual-channel asymmetric stereo courtyard reflections (L: 15/33 ms, R: 19/41 ms + cross-feed).
* **Smart Voice Stealing:** Replaced fixed voice-0 stealing with quietest-voice stealing for smoother 16-voice polyphony.
* **Nyquist & High Sample-Rate Safety:** Added frequency clamping on biquad filters and upper FM modulators, dynamic delay buffer sizing up to 192 kHz, and master volume smoothing.
* **MIDI Expressiveness:** Added full Sustain Pedal (CC 64) and Pitch Bend (±2 semitones) support, and increased `getTailLengthSeconds()` to `3.5s`.

### GUI, UX & Bug Fixes
* **Fixed Preset Index Bug:** Fixed user preset loading offset from `30` to `40` so factory presets 31–40 never collide with saved user presets.
* **Restored `SAVE` Button & Added `INIT` / `RND`:** Restored the visible `SAVE` button in the preset bar (with crash-safe modal callback) and added header `INIT` (clean concert marimba) and `RND` (musical patch randomizer) buttons plus a clickable `GPLv3 // FREE` badge.
* **Symmetrical `680×420` Layout & Knob UX:** Balanced all three modular sections (`208px` each), added hover highlights and **double-click to reset** on all 12 knobs, and added a live `PAD: C4` pitch badge to the Logo Trigger Pad.
* **Live LCD Telemetry:** Upgraded the 5 modal bars (`f0`, `4f`, `10f`, `TUBE`, `BUZZ`) to react dynamically to real-time RMS audio energy and synth parameters, and added a live `VOICES X/16` counter.
* **Build & Installer Fixes:** Fixed macOS post-build installation paths and cross-platform installer scripts to consistently use `Extasis Marimba` (with space).

## [3.1.0] - 2026-09-04
### DSP & Physics Overhaul
* **True 1:4:10 Wood Carving Engine:** Overhauled FM mode ratios to match hand-carved physical marimba tuning.
* **Acoustic Key-Tracked Panning & Decay:** Low notes pan left and ring; high notes pan right and decay rapidly.
* **Expanded Factory Banks:** Increased factory library from 30 to 40 categorized presets.

## [1.0.0] - 2026-08-22 (Initial Release)
* Initial Physical-FM Marimba synthesis engine, *La Cachimba* membrane buzz, *Marimba de Pueblo* drift, 30 presets, OLED oscilloscope, and Logo Trigger Pad.
