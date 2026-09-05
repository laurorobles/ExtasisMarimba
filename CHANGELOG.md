# 📜 Changelog — Extasis Marimba

# 📜 Changelog — Extasis Marimba

## [3.0.0] - 2026-09-04 (Major Final Release)
### DSP & Physics Overhaul
* **True 1:4:10 Wood Carving Engine:** Completely overhauled the FM mode ratios to perfectly match physical marimba tuning, ditching the uniform dissonant bar model.
* **Acoustic Key-Tracked Panning:** Implemented physical spatialization where low notes resonate on the left and high notes on the right, scaled by the `SPREAD` knob.
* **Aggressive Key-Tracked Decay:** Physical modeling enhancement where high notes decay instantly ("tock") while low notes ring out indefinitely.
* **Saturator:** Added a `std::tanh` soft-clipper at the master output for 1-3dB of warm analog acoustic glue.

### GUI & User Experience
* **Expanded Interface (660x410):** Significantly increased the plugin dimensions, providing luxurious margins around all modules and knobs.
* **Enhanced Preset Menu:** Widened the preset combobox so long preset names no longer cut off, and restored the dedicated `SAVE` button.
* **Unobtrusive Licensing:** Removed the "BUY LICENSE" button from the main header. The Gumroad link now strictly appears inside the Activation Overlay.
* **Developer Credit:** Added a sleek, discreet `"coded by @laurorobles"` watermark integrated into the main header.
* **DAW Naming Fix:** Updated CMake metadata so the plugin correctly shows up as `Extasis Marimba` (with a space) in Ableton Live and Logic.

### Presets
* **Expanded Factory Banks:** Increased the factory library from 30 to 40 categorized presets, meticulously mapped to the 12 new physical parameters (covering Traditional, Latin Club, Concert, and Experimental Hybrid FM).

## [1.0.0] - 2026-08-22 (Initial Release)
### Added
* 64-bit Modal Physical Modeling & FM Marimba synthesis engine.
* *La Cachimba* non-linear membrane buzz modeling with velocity sensitivity.
* Rubber mallet noise strike bandpass filter (2.5 kHz - 5.5 kHz).
* Acoustic resonator tube cavity model with micro-beating ($\pm 3.5\text{--}6\text{ Hz}$).
* *Marimba de Pueblo* deterministic per-key micro-imperfection engine.
* 30 professionally designed factory presets.
* User preset saving & loading system (`.empreset`).
* High-definition multi-mode OLED oscilloscope & modal resonator visualizer.
* Large tactile Extasis Logo Trigger Pad with drag-to-tune pitch auditioning ($\pm 24\text{ st}$).
* Standard Extasis Records offline cryptographic licensing system with in-app activation modal.
* Native Standalone, VST3, and Audio Unit support for macOS and Windows.
