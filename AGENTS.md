# 📜 Extasis Marimba — Engineering & Documentation Guidelines

## 1. Documentation Integrity Standard
Whenever any feature, DSP algorithm, parameter, preset, GUI layout, or build configuration is added, modified, or refactored in this codebase, keep the following documentation files synchronized in English:

1. **`MANUAL.md`** — User manual (controls, MIDI support, presets, installation).
2. **`ARCHITECTURE.md`** — Signal flow diagram, DSP math, and threading architecture.
3. **`TECHNICAL.md`** — Technical specs (polyphony, sample rates, formats, requirements).
4. **`CHANGELOG.md`** — Semantic version history.
5. **`README.md`** — Overview, features, badges, build commands, and download links.
6. **`package_release.sh` & `.github/workflows/build.yml`** — Release packaging & CI metadata.

## 2. Open Source License
- **Extasis Marimba** is 100% Free & Open Source under the **GNU General Public License v3.0 (GPLv3)**.
- Do not introduce proprietary DRM, serial activation gates, or demo time limits.

## 3. Cross-Platform Parity
- Maintain full compilation support for macOS (Apple Silicon & Intel), Windows (x64), and Linux (x64).
- Plugin product name must remain `"Extasis Marimba"` across all platforms and installers.
