#!/usr/bin/env bash
set -euo pipefail

# Ivan Valve Amplifier — install script
# Copies built plugins to the correct system locations per platform

BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"
ARTIFACTS="${BUILD_DIR}/Ivan_artefacts/${BUILD_TYPE}"

if [ ! -d "$ARTIFACTS" ]; then
    echo "ERROR: Build artifacts not found at ${ARTIFACTS}"
    echo "Run ./scripts/build.sh first."
    exit 1
fi

case "$(uname -s)" in
    Darwin)
        echo "=== Installing on macOS ==="

        # AU for Logic Pro, GarageBand, etc.
        AU_SRC="${ARTIFACTS}/AU/Ivan.component"
        AU_DST="$HOME/Library/Audio/Plug-Ins/Components"
        if [ -d "$AU_SRC" ]; then
            mkdir -p "$AU_DST"
            rm -rf "${AU_DST}/Ivan.component"
            cp -R "$AU_SRC" "$AU_DST/"
            echo "  AU installed:   ${AU_DST}/Ivan.component"
        fi

        # VST3 for Ableton, REAPER, Bitwig, etc.
        VST3_SRC="${ARTIFACTS}/VST3/Ivan.vst3"
        VST3_DST="$HOME/Library/Audio/Plug-Ins/VST3"
        if [ -d "$VST3_SRC" ]; then
            mkdir -p "$VST3_DST"
            rm -rf "${VST3_DST}/Ivan.vst3"
            cp -R "$VST3_SRC" "$VST3_DST/"
            echo "  VST3 installed: ${VST3_DST}/Ivan.vst3"
        fi

        # Standalone
        APP_SRC="${ARTIFACTS}/Standalone/Ivan.app"
        APP_DST="/Applications"
        if [ -d "$APP_SRC" ]; then
            rm -rf "${APP_DST}/Ivan.app"
            cp -R "$APP_SRC" "$APP_DST/"
            echo "  App installed:  ${APP_DST}/Ivan.app"
        fi

        echo ""
        echo "Done. Restart your DAW to pick up the new plugin."
        echo "In Logic Pro: open a channel strip > Audio FX > editsip > Ivan"
        ;;

    Linux)
        echo "=== Installing on Linux ==="

        # VST3
        VST3_SRC="${ARTIFACTS}/VST3/Ivan.vst3"
        VST3_DST="$HOME/.vst3"
        if [ -d "$VST3_SRC" ]; then
            mkdir -p "$VST3_DST"
            rm -rf "${VST3_DST}/Ivan.vst3"
            cp -R "$VST3_SRC" "$VST3_DST/"
            echo "  VST3 installed: ${VST3_DST}/Ivan.vst3"
        fi

        # Standalone
        STANDALONE_SRC="${ARTIFACTS}/Standalone/Ivan"
        STANDALONE_DST="$HOME/.local/bin"
        if [ -f "$STANDALONE_SRC" ]; then
            mkdir -p "$STANDALONE_DST"
            cp "$STANDALONE_SRC" "${STANDALONE_DST}/ivan"
            chmod +x "${STANDALONE_DST}/ivan"
            echo "  Standalone:     ${STANDALONE_DST}/ivan"
        fi

        echo ""
        echo "Done. Restart your DAW to pick up the new plugin."
        ;;

    MINGW*|MSYS*|CYGWIN*)
        echo "=== Installing on Windows ==="

        # VST3
        VST3_SRC="${ARTIFACTS}/VST3/Ivan.vst3"
        VST3_DST="$LOCALAPPDATA/Programs/Common/VST3"
        if [ -d "$VST3_SRC" ]; then
            mkdir -p "$VST3_DST"
            rm -rf "${VST3_DST}/Ivan.vst3"
            cp -R "$VST3_SRC" "$VST3_DST/"
            echo "  VST3 installed: ${VST3_DST}/Ivan.vst3"
        fi

        # Also try the common C:\Program Files path via symlink-friendly location
        VST3_ALT="/c/Program Files/Common Files/VST3"
        if [ -d "$VST3_ALT" ]; then
            rm -rf "${VST3_ALT}/Ivan.vst3"
            cp -R "$VST3_SRC" "$VST3_ALT/" 2>/dev/null || echo "  (Need admin for Program Files install)"
        fi

        # Standalone
        STANDALONE_SRC="${ARTIFACTS}/Standalone/Ivan.exe"
        STANDALONE_DST="$LOCALAPPDATA/Programs/Ivan"
        if [ -f "$STANDALONE_SRC" ]; then
            mkdir -p "$STANDALONE_DST"
            cp "$STANDALONE_SRC" "$STANDALONE_DST/"
            echo "  Standalone:     ${STANDALONE_DST}/Ivan.exe"
        fi

        echo ""
        echo "Done. Restart your DAW to pick up the new plugin."
        ;;

    *)
        echo "Unknown platform: $(uname -s)"
        exit 1
        ;;
esac
