#!/bin/bash
set -e

# ==============================================================================
# Android SDK/NDK Setup Script (No Android Studio Required)
# ==============================================================================

# Directory where we will install the SDK (local to project)
PROJECT_ROOT="$(pwd)"
SDK_DIR="$PROJECT_ROOT/libs/android-sdk"
CMDLINE_TOOLS_ZIP="commandlinetools-linux-11076708_latest.zip"
DOWNLOAD_URL="https://dl.google.com/android/repository/$CMDLINE_TOOLS_ZIP"

# 1. Check for Java (Required for SDK tools)
if ! command -v java &> /dev/null; then
    echo "[Error] Java (JDK) is not installed."
    echo "  Please install it first (e.g., sudo pacman -S jdk-openjdk or sudo apt install default-jdk)."
    exit 1
fi

echo "[Setup] Installing Android SDK to: $SDK_DIR"
mkdir -p "$SDK_DIR"

# 2. Download Command Line Tools
if [ ! -d "$SDK_DIR/cmdline-tools" ]; then
    echo "[Setup] Downloading Command Line Tools..."
    wget -q --show-progress "$DOWNLOAD_URL" -O "$SDK_DIR/$CMDLINE_TOOLS_ZIP"
    
    echo "[Setup] Extracting..."
    unzip -q "$SDK_DIR/$CMDLINE_TOOLS_ZIP" -d "$SDK_DIR"
    
    # Reorganize to standard structure: cmdline-tools/latest/bin
    mkdir -p "$SDK_DIR/cmdline-tools/latest"
    mv "$SDK_DIR/cmdline-tools/bin" "$SDK_DIR/cmdline-tools/latest/"
    mv "$SDK_DIR/cmdline-tools/lib" "$SDK_DIR/cmdline-tools/latest/"
    mv "$SDK_DIR/cmdline-tools/source.properties" "$SDK_DIR/cmdline-tools/latest/"
    
    rm "$SDK_DIR/$CMDLINE_TOOLS_ZIP"
else
    echo "[Setup] Command Line Tools already installed."
fi

# 3. Setup Environment Variables for Installation
export ANDROID_HOME="$SDK_DIR"
export PATH="$ANDROID_HOME/cmdline-tools/latest/bin:$PATH"

# 4. Install Required Components (SDK 33, NDK, CMake)
echo "[Setup] Installing SDK Components, NDK, and CMake..."
echo "[Info] You may need to accept licenses by typing 'y'."

# "yes" automatically accepts licenses
yes | sdkmanager --licenses > /dev/null 2>&1
yes | sdkmanager "platform-tools" "platforms;android-33" "build-tools;33.0.1" "ndk;25.2.9519653" "cmake;3.22.1"

echo ""
echo "========================================================"
echo "  [SUCCESS] Android SDK & NDK Installed!"
echo "========================================================"
echo ""
echo "  To build the game, you must set these variables."
echo "  Run this command in your terminal before building:"
echo ""
echo "  export ANDROID_HOME=\"$SDK_DIR\""
echo "  export ANDROID_NDK_HOME=\"$SDK_DIR/ndk/25.2.9519653\""
echo ""