# 📋 Release Rules — Extasis Marimba

This document defines the versioning, documentation, and build rules for every official open-source release.

## 1. Versioning & Metadata
* **Semantic Versioning:** Strictly follow `MAJOR.MINOR.PATCH` (e.g. `4.0.0`).
* **`CMakeLists.txt`:** `project(ExtasisMarimba VERSION X.Y.Z)` must match the Git tag `vX.Y.Z`.
* **Documentation:** `README.md`, `MANUAL.md`, `TECHNICAL.md`, `ARCHITECTURE.md`, `PRESENTATION.md`, and `CHANGELOG.md` must all be up to date in English.

## 2. Release Workflow
* **macOS:** Built and packaged locally (`package_release.sh`) into `dist/ExtasisMarimba-macOS.zip` and uploaded to the GitHub Release.
* **Windows & Linux:** Automatically compiled and attached to the GitHub Release via `.github/workflows/build.yml` when pushing a `vX.Y.Z` tag.
* **License:** All packages include the GNU GPLv3 `LICENSE` file.
