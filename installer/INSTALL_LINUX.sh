#!/bin/bash
echo "============================================================"
echo " Extasis Marimba - Linux Automated Installer"
echo "============================================================"

INSTALL_DIR="$(cd "$(dirname "$0")" && pwd)"
VST3_DIR="$HOME/.vst3"
CLAP_DIR="$HOME/.clap"

echo "[1/2] Installing VST3 Plugin to $VST3_DIR..."
mkdir -p "$VST3_DIR"
if [ -d "$INSTALL_DIR/Extasis Marimba.vst3" ]; then
    rm -rf "$VST3_DIR/ExtasisMarimba.vst3" "$VST3_DIR/Extasis Marimba.vst3"
    cp -R "$INSTALL_DIR/Extasis Marimba.vst3" "$VST3_DIR/"
elif [ -d "$INSTALL_DIR/../Extasis Marimba.vst3" ]; then
    rm -rf "$VST3_DIR/ExtasisMarimba.vst3" "$VST3_DIR/Extasis Marimba.vst3"
    cp -R "$INSTALL_DIR/../Extasis Marimba.vst3" "$VST3_DIR/"
fi

echo "[2/2] Installing CLAP Plugin to $CLAP_DIR..."
mkdir -p "$CLAP_DIR"
if [ -f "$INSTALL_DIR/Extasis Marimba.clap" ]; then
    cp "$INSTALL_DIR/Extasis Marimba.clap" "$CLAP_DIR/"
elif [ -f "$INSTALL_DIR/../Extasis Marimba.clap" ]; then
    cp "$INSTALL_DIR/../Extasis Marimba.clap" "$CLAP_DIR/"
fi

echo ""
echo "============================================================"
echo " Installation Complete! Open your DAW and rescan plugins."
echo "============================================================"
