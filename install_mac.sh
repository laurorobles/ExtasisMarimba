#!/bin/bash
set -e

echo "🪵 Installing Extasis Marimba for macOS..."

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Target directories
VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"
APP_DIR="/Applications"
PRESETS_DIR="$HOME/Documents/ExtasisRecords/ExtasisMarimba/Presets"

mkdir -p "$VST3_DIR"
mkdir -p "$AU_DIR"
mkdir -p "$PRESETS_DIR"

# Install VST3
if [ -d "$SCRIPT_DIR/Extasis Marimba.vst3" ]; then
    echo "  -> Copying Extasis Marimba.vst3 to $VST3_DIR..."
    rm -rf "$VST3_DIR/Extasis Marimba.vst3"
    cp -R "$SCRIPT_DIR/Extasis Marimba.vst3" "$VST3_DIR/"
fi

# Install AU
if [ -d "$SCRIPT_DIR/Extasis Marimba.component" ]; then
    echo "  -> Copying Extasis Marimba.component to $AU_DIR..."
    rm -rf "$AU_DIR/Extasis Marimba.component"
    cp -R "$SCRIPT_DIR/Extasis Marimba.component" "$AU_DIR/"
fi

# Install Standalone App
if [ -d "$SCRIPT_DIR/Extasis Marimba.app" ]; then
    echo "  -> Copying Extasis Marimba.app to $APP_DIR..."
    rm -rf "$APP_DIR/Extasis Marimba.app"
    cp -R "$SCRIPT_DIR/Extasis Marimba.app" "$APP_DIR/"
fi

# Reset macOS AudioComponent cache if auval is present
if command -v killall &> /dev/null; then
    killall -9 AudioComponentRegistrar 2>/dev/null || true
fi

echo "✅ Extasis Marimba successfully installed on macOS!"
