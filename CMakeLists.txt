cmake_minimum_required(VERSION 3.16)
project(2048-Core C)

# ==============================================================================
# 1. Global Configuration
# ==============================================================================
# Generate compile_commands.json for LSP/Intellisense support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==============================================================================
# 2. Dependency: SDL2 (Static Build)
# ==============================================================================
# We configure SDL2 BEFORE adding the subdirectory.
# Note: We do NOT set global C90 flags here, so SDL2 builds with modern C.
set(SDL_SHARED OFF CACHE BOOL "Disable shared SDL2 lib" FORCE)
set(SDL_STATIC ON CACHE BOOL "Enable static SDL2 lib" FORCE)
set(SDL_TEST OFF CACHE BOOL "Disable SDL2 test code" FORCE)
set(SDL2_DISABLE_INSTALL ON CACHE BOOL "Don't install SDL2 system-wide" FORCE)

# FIX: Disable PipeWire to prevent compilation errors with newer system headers
set(SDL_PIPEWIRE OFF CACHE BOOL "Disable PipeWire Audio" FORCE)

add_subdirectory(libs/SDL2)

# ==============================================================================
# 3. Target: Main Game Executable
# ==============================================================================
# Gather sources
file(GLOB_RECURSE PROJECT_SOURCES "src/*.c")

add_executable(2048_core ${PROJECT_SOURCES})

# Include directories
target_include_directories(2048_core PUBLIC 
    include
    libs/SDL2/include
)

# --- STRICT ANSI C89 ENFORCEMENT (Target Specific) ---
# This applies C90 ONLY to your game code, protecting SDL2 from errors.
set_target_properties(2048_core PROPERTIES
    C_STANDARD 90
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

# ==============================================================================
# 4. Compiler Hardening ("The Wall of Safety")
# ==============================================================================
target_compile_options(2048_core PRIVATE
    -Wall 
    -Wextra 
    -Wpedantic 
    -Wshadow 
    -Wconversion 
    -Wstrict-prototypes
)

# Debug Configuration
target_compile_options(2048_core PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
)

target_link_options(2048_core PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

# Release Configuration
target_compile_options(2048_core PRIVATE
    $<$<CONFIG:Release>:-O3>
)

# Enable Link Time Optimization (LTO)
set_property(TARGET 2048_core PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

# ==============================================================================
# 5. Linking
# ==============================================================================
target_link_libraries(2048_core PRIVATE 
    SDL2-static 
    m 
    dl 
    pthread
)

# ==============================================================================
# 6. Post-Build: Asset Management
# ==============================================================================
add_custom_command(TARGET 2048_core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory..."
)

# ==============================================================================
# 7. Core Logic Library (Decoupled)
# ==============================================================================
add_library(core_logic STATIC src/game_logic.c)
target_include_directories(core_logic PUBLIC include)

# Apply Strict C89 to the library as well
set_target_properties(core_logic PROPERTIES
    C_STANDARD 90
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

# ==============================================================================
# 8. Unit Testing
# ==============================================================================
add_executable(run_tests tests/test_logic.c)

# Link core logic (No SDL)
target_link_libraries(run_tests PRIVATE core_logic)
target_compile_definitions(run_tests PRIVATE TEST_MODE)

# Apply Strict C89 to the tests as well
set_target_properties(run_tests PROPERTIES
    C_STANDARD 90
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

add_custom_target(test_logic 
    COMMAND run_tests
    DEPENDS run_tests
    COMMENT "Running Logic Unit Tests..."
)