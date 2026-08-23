---
trigger: always_on
description: Mandatory rule to always update all project documentation, manuals, architecture, specs, and changelogs whenever changes occur.
---

# Documentation & Engineering Rule

Whenever any code, DSP algorithm, parameter, knob, preset, GUI layout, or build script is created or modified:
1. **`MANUAL.md`**: Update knob explanations, preset listings, and user instructions.
2. **`ARCHITECTURE.md`**: Update signal block diagrams, DSP equations, and thread concurrency documentation.
3. **`SPECIFICATIONS.md`**: Keep latency, polyphony, sample rate, and DAW support current.
4. **`CHANGELOG.md`**: Log all additions, fixes, and changes.
5. **`README.md`**: Keep feature highlights, quick-start, and build commands synchronized.
6. **`package_release.sh`**: Re-bundle latest docs into `dist/`.
