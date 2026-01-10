#!/bin/bash

# Usage: ./scripts/build_local.sh [debug|release]

BUILD_TYPE=${1:-debug} # Default to debug if no argument provided
BUILD_DIR="build"

# Normalize input to lowercase
BUILD_TYPE=$(echo "$BUILD_TYPE" | tr '[:upper:]' '[:lower:]')

# Determine CMake Build Type String
if [ "$BUILD_TYPE" == "release" ]; then
    CMAKE_BUILD_TYPE="Release"
elif [ "$BUILD_TYPE" == "debug" ]; then
    CMAKE_BUILD_TYPE="Debug"
else
    echo "Error: Invalid build type '$BUILD_TYPE'. Use 'debug' or 'release'."
    exit 1
fi

echo "============================================"
echo "Building 2048-Core ($CMAKE_BUILD_TYPE)"
echo "============================================"

# Create build directory if needed
mkdir -p "$BUILD_DIR"

# Configure CMake
# -S = Source Dir, -B = Build Dir
cmake -S . -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE

# Build with parallelism (uses all available cores)
cmake --build "$BUILD_DIR" --parallel $(nproc)

echo ""
echo "Build Complete. Executable location:"
echo "$BUILD_DIR/2048_core"