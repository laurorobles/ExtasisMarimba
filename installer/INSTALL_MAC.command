#!/bin/bash
echo "============================================================"
echo " ExtasisMarimba - macOS Automated Installer"
echo "============================================================"

INSTALL_DIR="$(cd "$(dirname "$0")" && pwd)"
VST3_DIR="/Library/Audio/Plug-Ins/VST3"
AU_DIR="/Library/Audio/Plug-Ins/Components"

echo "[1/2] Installing VST3 Plugin to $VST3_DIR..."
sudo mkdir -p "$VST3_DIR"
if [ -d "$INSTALL_DIR/ExtasisMarimba.vst3" ]; then
    sudo cp -R "$INSTALL_DIR/ExtasisMarimba.vst3" "$VST3_DIR/"
fi

echo "[2/2] Installing AU Plugin to $AU_DIR..."
sudo mkdir -p "$AU_DIR"
if [ -d "$INSTALL_DIR/ExtasisMarimba.component" ]; then
    sudo cp -R "$INSTALL_DIR/ExtasisMarimba.component" "$AU_DIR/"
fi

echo ""
echo "============================================================"
echo " Installation Complete! Open your DAW and rescan plugins."
echo "============================================================"
