# 2048-Core

![Build Status](https://img.shields.io/badge/build-passing-brightgreen)
![License](https://img.shields.io/badge/license-MIT-blue)
![Standard](https://img.shields.io/badge/standard-C89-orange)

A professional, cross-platform implementation of the 2048 game engine written in strict ANSI C (C89).

## Project Architecture

The project follows a strict monorepo structure designed for portability:

* **`src/`**: Core game logic and implementation files (`.c`).
* **`include/`**: Public API headers (`.h`).
* **`libs/`**: Vendored third-party dependencies (SDL2).
* **`android/`**: Android NDK build configuration and scaffolding.
* **`assets/`**: Binary assets, textures, and fonts.
* **`tests/`**: Unit testing suite.

## Build Instructions

### Prerequisites
* CMake 3.10+
* GCC or Clang
* Make or Ninja

### Linux (Manjaro)
```bash
mkdir build && cd build
cmake ..
make
./2048-Core
```

### Android
Refer to `android/README.md` (to be created) for NDK build steps.
