#ifndef STORAGE_H
#define STORAGE_H

/* ==============================================================================
 * Project: 2048-Core
 * File: include/storage.h
 * Standard: ANSI C (C89)
 * Description: Binary persistence layer with versioning and checksums.
 * ============================================================================== */

#include "game_logic.h"

/* Return Codes */
#define STORAGE_OK 0
#define STORAGE_ERR_OPEN 1
#define STORAGE_ERR_NOT_FOUND 2
#define STORAGE_ERR_VERSION 3
#define STORAGE_ERR_CORRUPT 4
#define STORAGE_ERR_WRITE 5

/**
 * @brief Saves the game state to disk atomically.
 * * 1. Writes to temporary file.
 * 2. Verifies write success.
 * 3. Renames temporary file to actual file (Atomic commit).
 * * @param base_path The directory to save in ("." for local, or Android internal path).
 * @param state The game state to serialize.
 * @return STORAGE_OK on success, or error code.
 */
int storage_save(const char *base_path, const GameState *state);

/**
 * @brief Loads the game state from disk.
 * * Verifies Magic Number, Version, and Checksum.
 * * @param base_path The directory to load from.
 * @param state Pointer to target struct to fill.
 * @return STORAGE_OK on success, or error code.
 */
int storage_load(const char *base_path, GameState *state);

#endif /* STORAGE_H */