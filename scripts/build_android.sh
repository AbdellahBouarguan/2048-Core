#!/bin/bash
set -e

# ==============================================================================
# Android Build Script for 2048-Core (Auto-Bootstrapping)
# ==============================================================================

PROJECT_ROOT="$(pwd)"
ANDROID_PROJECT_DIR="$PROJECT_ROOT/android"
LOCAL_SDK="$PROJECT_ROOT/libs/android-sdk"

# 1. Environment Verification
# ------------------------------------------------------------------------------
if [ -z "$ANDROID_HOME" ]; then
    if [ -d "$LOCAL_SDK" ]; then
        export ANDROID_HOME="$LOCAL_SDK"
        echo "[Info] Using local Android SDK: $ANDROID_HOME"
    else
        echo "[Error] ANDROID_HOME is not set and no local SDK found."
        echo "  Please run './scripts/install_android_tools.sh' first."
        exit 1
    fi
fi

if [ -z "$ANDROID_NDK_HOME" ]; then
    NDK_DIR=$(ls -d "$ANDROID_HOME/ndk/"* 2>/dev/null | tail -n 1)
    if [ -n "$NDK_DIR" ]; then
        export ANDROID_NDK_HOME="$NDK_DIR"
        echo "[Info] Using NDK: $ANDROID_NDK_HOME"
    else
        echo "[Error] NDK not found in $ANDROID_HOME/ndk"
        exit 1
    fi
fi

# 2. SDL2 Java Source Setup
# ------------------------------------------------------------------------------
SDL_JAVA_SRC_DIR="libs/SDL2/android-project/app/src/main/java/org/libsdl/app"
DEST_DIR="android/app/src/main/java/org/libsdl/app"
DEST_FILE="$DEST_DIR/SDLActivity.java"

if [ ! -f "$DEST_FILE" ]; then
    echo "[Setup] Copying SDL2 Java sources..."
    if [ -d "$SDL_JAVA_SRC_DIR" ]; then
        mkdir -p "$DEST_DIR"
        cp -r "$SDL_JAVA_SRC_DIR/"* "$DEST_DIR/"
    else
        echo "[Error] Could not find SDL2 Java sources in $SDL_JAVA_SRC_DIR"
        exit 1
    fi
fi

# 3. Gradle Bootstrapping (Updated for Java 21)
# ------------------------------------------------------------------------------
if [ ! -f "$ANDROID_PROJECT_DIR/gradlew" ]; then
    echo "[Setup] 'gradlew' not found. Bootstrapping Gradle..."
    
    # [UPDATED] Use Gradle 8.5 which supports Java 21
    GRADLE_VER="8.5"
    GRADLE_ZIP="gradle-$GRADLE_VER-bin.zip"
    URL="https://services.gradle.org/distributions/$GRADLE_ZIP"
    TEMP_DIR="$PROJECT_ROOT/libs/gradle_temp"
    
    mkdir -p "$TEMP_DIR"
    
    if [ ! -f "$TEMP_DIR/$GRADLE_ZIP" ]; then
        echo "[Setup] Downloading Gradle $GRADLE_VER..."
        wget -q --show-progress "$URL" -O "$TEMP_DIR/$GRADLE_ZIP"
    fi
    
    if [ ! -d "$TEMP_DIR/gradle-$GRADLE_VER" ]; then
        echo "[Setup] Extracting Gradle..."
        unzip -q "$TEMP_DIR/$GRADLE_ZIP" -d "$TEMP_DIR"
    fi
    
    echo "[Setup] Generating Gradle Wrapper..."
    "$TEMP_DIR/gradle-$GRADLE_VER/bin/gradle" -p "$ANDROID_PROJECT_DIR" wrapper
    
    echo "[Setup] Wrapper generated."
fi

# 4. Build Process
# ------------------------------------------------------------------------------
echo "[Build] Starting Android Build (Debug)..."

cd "$ANDROID_PROJECT_DIR"
chmod +x ./gradlew
./gradlew assembleDebug

# 5. Success Message
# ------------------------------------------------------------------------------
if [ $? -eq 0 ]; then
    echo ""
    echo "========================================================"
    echo "  [SUCCESS] Android Build Complete"
    echo "  APK Location: android/app/build/outputs/apk/debug/app-debug.apk"
    echo "========================================================"
else
    echo "[Error] Build failed."
    exit 1
fi