#!/bin/bash
set -e

echo "📦 Packaging Extasis Marimba v1.0.0 Release Packages..."

PROJECT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$PROJECT_DIR/build/ExtasisMarimba_artefacts/Release"
DIST_DIR="$PROJECT_DIR/dist"

mkdir -p "$DIST_DIR"

# 1. macOS Package
MAC_DIR="$DIST_DIR/ExtasisMarimba_v1.0.0_macOS"
rm -rf "$MAC_DIR"
mkdir -p "$MAC_DIR"

cp -R "$BUILD_DIR/Standalone/ExtasisMarimba.app" "$MAC_DIR/"
cp -R "$BUILD_DIR/VST3/ExtasisMarimba.vst3" "$MAC_DIR/"
cp -R "$BUILD_DIR/AU/ExtasisMarimba.component" "$MAC_DIR/"
cp "$PROJECT_DIR/install_mac.sh" "$MAC_DIR/"
cp "$PROJECT_DIR/README.md" "$MAC_DIR/"
cp "$PROJECT_DIR/MANUAL.md" "$MAC_DIR/"

cd "$DIST_DIR"
zip -r -q "ExtasisMarimba_v1.0.0_macOS.zip" "ExtasisMarimba_v1.0.0_macOS"
echo "✅ Created ExtasisMarimba_v1.0.0_macOS.zip"

# 2. Windows Release Setup
WIN_DIR="$DIST_DIR/ExtasisMarimba_v1.0.0_Windows"
rm -rf "$WIN_DIR"
mkdir -p "$WIN_DIR"

cp -R "$BUILD_DIR/VST3/ExtasisMarimba.vst3" "$WIN_DIR/"
cp "$PROJECT_DIR/install_windows.bat" "$WIN_DIR/"
cp "$PROJECT_DIR/README.md" "$WIN_DIR/"
cp "$PROJECT_DIR/MANUAL.md" "$WIN_DIR/"

zip -r -q "ExtasisMarimba_v1.0.0_Windows.zip" "ExtasisMarimba_v1.0.0_Windows"
echo "✅ Created ExtasisMarimba_v1.0.0_Windows.zip"

echo "🎉 All release packages generated in $DIST_DIR!"
