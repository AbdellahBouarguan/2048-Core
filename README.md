Here is the complete `README.md` in a single copy-paste block.


# 2048-Core

![Standard](https://img.shields.io/badge/standard-C89-orange)
![License](https://img.shields.io/badge/license-MIT-blue)
![Platform](https://img.shields.io/badge/platform-Linux%20|%20Android-green)

A high-performance, cross-platform implementation of the 2048 game engine written in strict ANSI C (C89).

## 🏗 Project Architecture

The project utilizes a monorepo structure designed for zero-dependency portability.

```text
2048-Core/
├── src/            # Core implementation (Strict C89)
│   ├── main.c      # Entry point & Event Loop
│   ├── game_logic.c# Mathematical core (No SDL dependencies)
│   ├── renderer.c  # Procedural SDL2 rendering
│   └── storage.c   # Atomic binary serialization
├── include/        # Public API headers
├── libs/           # Vendored dependencies
│   └── SDL2/       # SDL2 source (compiled statically)
├── assets/         # (Optional) External assets
├── build/          # Build artifacts
└── scripts/        # Automation scripts
```


## 🚀 Build Instructions

### Prerequisites

- **Linux**: `cmake`, `make`, `gcc`
- **Android**: Android NDK (for cross-compilation)

### Quick Start (Linux)

The included script handles directory creation and CMake configuration automatically.

```bash
# Build Debug version
./scripts/build_local.sh debug

# Run
./build/2048_core

```

### CLI Options

- `--reset`: Deletes the save file and starts a fresh game.
- `--version`: Prints the current version.

## 📐 Design Philosophy

1. **Strict C89**: The codebase adheres to the ANSI C standard (ISO/IEC 9899:1990). This ensures the game can compile on virtually any platform, from modern desktops to embedded devices and legacy consoles.
2. **Zero External Dependencies**: SDL2 is vendored and compiled statically. The project does not rely on system shared libraries (except standard C libs), making the binary portable "drag-and-drop".
3. **Decoupled Logic**: The `game_logic` module is mathematically pure and has no knowledge of the rendering engine. This allows for headless testing and easy porting to other engines (e.g., Unity/Godot via plugins).
4. **Procedural Rendering**: Graphics are generated at runtime using vector math, ensuring crisp visuals at any resolution without managing sprite assets.

```

```
