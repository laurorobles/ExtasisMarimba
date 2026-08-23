# 📜 Extasis Records — Mandatory Engineering & Documentation Rules

## 1. Documentation Integrity Standard (ALWAYS ENFORCED)
Whenever any feature, DSP algorithm, parameter, preset, GUI layout, license logic, or build configuration is added, modified, or refactored in this codebase, the agent MUST ALWAYS update and synchronize the following documentation files without needing to be reminded:

1. **`MANUAL.md` (Official User Manual):**
   - Must document all parameters, knobs, values, signal flow, sound design philosophy, preset catalogs, and user workflows.
2. **`ARCHITECTURE.md` (Signal Architecture & DSP Engineering):**
   - Must document the exact block diagrams, mathematical formulas, physical modeling equations, thread concurrency, and signal routing.
3. **`SPECIFICATIONS.md` (Technical Specs):**
   - Must maintain up-to-date audio engine specs (polyphony, sample rates, latency, OS/DAW compatibility).
4. **`CHANGELOG.md` (Version History):**
   - Must log every change, new feature, DSP upgrade, and bugfix under semantic version headings.
5. **`README.md` (Repository Overview):**
   - Must keep features, build steps, badges, and quick-start installation guides in sync.
6. **`package_release.sh` & Distribution (`dist/`):**
   - Must ensure all updated documentation files are packaged into the release ZIP bundles.

## 2. Audio & License Confidentiality
- Keep cryptographic salt implementations and internal validation routines private and discreet in the public `README.md`.
- Ensure offline serial validation follows the standard Extasis Records licensing model (`EXTM-XXXX-XXXX-XXXX-XXXX`).

## 3. Cross-Platform Parity
- Always maintain full parity and compilation support for macOS (ARM64 Apple Silicon & x86_64 Intel) and Windows (64-bit).
- Build targets: Standalone (.app/.exe), VST3 (.vst3), and Audio Unit (.component).
