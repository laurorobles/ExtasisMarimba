#!/bin/bash
set -e

VERSION="4.0.0"
echo "📦 Packaging Extasis Marimba v${VERSION} (Free & Open Source) for macOS..."

PROJECT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$PROJECT_DIR/build/ExtasisMarimba_artefacts/Release"
DIST_DIR="$PROJECT_DIR/dist"

mkdir -p "$DIST_DIR"

# macOS Package
MAC_DIR="$DIST_DIR/ExtasisMarimba-macOS"
rm -rf "$MAC_DIR" "$DIST_DIR/ExtasisMarimba-macOS.zip"
mkdir -p "$MAC_DIR"

cp -R "$BUILD_DIR/Standalone/Extasis Marimba.app" "$MAC_DIR/"
cp -R "$BUILD_DIR/VST3/Extasis Marimba.vst3" "$MAC_DIR/"
cp -R "$BUILD_DIR/AU/Extasis Marimba.component" "$MAC_DIR/"
cp "$PROJECT_DIR/install_mac.sh" "$MAC_DIR/"
cp "$PROJECT_DIR/LICENSE" "$MAC_DIR/"
cp "$PROJECT_DIR/README.md" "$MAC_DIR/"
cp "$PROJECT_DIR/MANUAL.md" "$MAC_DIR/"

cd "$DIST_DIR"
zip -r -q "ExtasisMarimba-macOS.zip" "ExtasisMarimba-macOS"
echo "✅ Created $DIST_DIR/ExtasisMarimba-macOS.zip"
