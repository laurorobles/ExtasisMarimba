#!/bin/bash
echo "============================================================"
echo " Extasis Marimba - macOS Automated Installer"
echo "============================================================"

INSTALL_DIR="$(cd "$(dirname "$0")" && pwd)"
VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"

echo "[1/2] Installing VST3 Plugin to $VST3_DIR..."
mkdir -p "$VST3_DIR"
if [ -d "$INSTALL_DIR/Extasis Marimba.vst3" ]; then
    rm -rf "$VST3_DIR/ExtasisMarimba.vst3" "$VST3_DIR/Extasis Marimba.vst3"
    cp -R "$INSTALL_DIR/Extasis Marimba.vst3" "$VST3_DIR/"
fi

echo "[2/2] Installing AU Plugin to $AU_DIR..."
mkdir -p "$AU_DIR"
if [ -d "$INSTALL_DIR/Extasis Marimba.component" ]; then
    rm -rf "$AU_DIR/ExtasisMarimba.component" "$AU_DIR/Extasis Marimba.component"
    cp -R "$INSTALL_DIR/Extasis Marimba.component" "$AU_DIR/"
fi

echo ""
echo "============================================================"
echo " Installation Complete! Open your DAW and rescan plugins."
echo "============================================================"
