#!/usr/bin/env bash
set -euo pipefail

# Ivan Valve Amplifier — build script
# Works on macOS, Linux, and Windows (via Git Bash / MSYS2)

BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"

echo "=== Ivan Valve Amplifier ==="
echo "Platform: $(uname -s)"
echo "Build type: ${BUILD_TYPE}"
echo ""

# Platform-specific dependency check
case "$(uname -s)" in
    Darwin)
        echo "macOS detected — Xcode command line tools required"
        if ! xcode-select -p &>/dev/null; then
            echo "ERROR: Install Xcode CLI tools: xcode-select --install"
            exit 1
        fi
        ;;
    Linux)
        echo "Linux detected — checking dependencies..."
        MISSING=""
        for pkg in libasound2-dev libfreetype-dev libgtk-3-dev libxrandr-dev libxinerama-dev libxcursor-dev; do
            if ! dpkg -s "$pkg" &>/dev/null; then
                MISSING="$MISSING $pkg"
            fi
        done
        if [ -n "$MISSING" ]; then
            echo "Missing packages:$MISSING"
            echo "Install with: sudo apt-get install$MISSING"
            exit 1
        fi
        ;;
    MINGW*|MSYS*|CYGWIN*)
        echo "Windows detected — Visual Studio 2019+ required"
        ;;
esac

echo ""
echo "Configuring..."
cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo ""
echo "Building..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j "$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo ""
echo "=== Build complete ==="
echo ""

# Show output locations
case "$(uname -s)" in
    Darwin)
        echo "Artifacts:"
        echo "  AU:         ${BUILD_DIR}/Ivan_artefacts/${BUILD_TYPE}/AU/Ivan.component"
        echo "  VST3:       ${BUILD_DIR}/Ivan_artefacts/${BUILD_TYPE}/VST3/Ivan.vst3"
        echo "  Standalone: ${BUILD_DIR}/Ivan_artefacts/${BUILD_TYPE}/Standalone/Ivan.app"
        echo ""
        echo "Run ./scripts/install.sh to install to system plugin folders."
        ;;
    Linux)
        echo "Artifacts:"
        echo "  VST3:       ${BUILD_DIR}/Ivan_artefacts/${BUILD_TYPE}/VST3/Ivan.vst3"
        echo "  Standalone: ${BUILD_DIR}/Ivan_artefacts/${BUILD_TYPE}/Standalone/Ivan"
        echo ""
        echo "Run ./scripts/install.sh to install to system plugin folders."
        ;;
    MINGW*|MSYS*|CYGWIN*)
        echo "Artifacts:"
        echo "  VST3:       ${BUILD_DIR}/Ivan_artefacts/${BUILD_TYPE}/VST3/Ivan.vst3"
        echo "  Standalone: ${BUILD_DIR}/Ivan_artefacts/${BUILD_TYPE}/Standalone/Ivan.exe"
        echo ""
        echo "Run ./scripts/install.sh to install to system plugin folders."
        ;;
esac
