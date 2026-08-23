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
if [ -d "$SCRIPT_DIR/ExtasisMarimba.vst3" ]; then
    echo "  -> Copying ExtasisMarimba.vst3 to $VST3_DIR..."
    rm -rf "$VST3_DIR/ExtasisMarimba.vst3"
    cp -R "$SCRIPT_DIR/ExtasisMarimba.vst3" "$VST3_DIR/"
fi

# Install AU
if [ -d "$SCRIPT_DIR/ExtasisMarimba.component" ]; then
    echo "  -> Copying ExtasisMarimba.component to $AU_DIR..."
    rm -rf "$AU_DIR/ExtasisMarimba.component"
    cp -R "$SCRIPT_DIR/ExtasisMarimba.component" "$AU_DIR/"
fi

# Install Standalone App
if [ -d "$SCRIPT_DIR/ExtasisMarimba.app" ]; then
    echo "  -> Copying ExtasisMarimba.app to $APP_DIR..."
    rm -rf "$APP_DIR/ExtasisMarimba.app"
    cp -R "$SCRIPT_DIR/ExtasisMarimba.app" "$APP_DIR/"
fi

# Reset macOS AudioComponent cache if auval is present
if command -v killall &> /dev/null; then
    killall -9 AudioComponentRegistrar 2>/dev/null || true
fi

echo "✅ Extasis Marimba successfully installed on macOS!"
