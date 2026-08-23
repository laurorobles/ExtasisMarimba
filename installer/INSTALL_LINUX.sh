#!/bin/bash
echo "============================================================"
echo " ExtasisMarimba - Linux Automated Installer"
echo "============================================================"

INSTALL_DIR="$(cd "$(dirname "$0")" && pwd)"
VST3_DIR="$HOME/.vst3"
CLAP_DIR="$HOME/.clap"

echo "[1/2] Installing VST3 Plugin to $VST3_DIR..."
mkdir -p "$VST3_DIR"
if [ -d "$INSTALL_DIR/ExtasisMarimba.vst3" ]; then
    cp -R "$INSTALL_DIR/ExtasisMarimba.vst3" "$VST3_DIR/"
fi

echo "[2/2] Installing CLAP Plugin to $CLAP_DIR..."
mkdir -p "$CLAP_DIR"
if [ -f "$INSTALL_DIR/ExtasisMarimba.clap" ]; then
    cp "$INSTALL_DIR/ExtasisMarimba.clap" "$CLAP_DIR/"
fi

echo ""
echo "============================================================"
echo " Installation Complete! Open your DAW and rescan plugins."
echo "============================================================"
