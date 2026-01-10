#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

/* ==============================================================================
 * Project: 2048-Core
 * File: include/game_logic.h
 * Standard: ANSI C (C89)
 * Description: Core data structures and API definitions.
 * ============================================================================== */

/* Game Status Enumeration */
typedef enum { GAME_ACTIVE = 0, GAME_WON = 1, GAME_OVER = 2 } GameStatus;

/* Movement Directions */
typedef enum { DIR_UP = 0, DIR_DOWN = 1, DIR_LEFT = 2, DIR_RIGHT = 3 } Direction;

/**
 * @struct GameState
 * @brief Holds the entire state of the 2048 game session.
 *
 * @var board 1D array representing 4x4 grid. Index = row * 4 + col.
 * @var score Current player score.
 * @var status Current game status flag.
 */
typedef struct {
    int board[16];
    unsigned long score;
    int status;
} GameState;

/* ==============================================================================
 * Public API
 * ============================================================================== */

/**
 * @brief Resets the board to all zeros, score to 0, and status to ACTIVE.
 * @param state Pointer to the GameState object.
 */
void game_init(GameState *state);

/**
 * @brief Spawns a new tile (2 or 4) in a random empty slot.
 * If TEST_MODE is defined, behavior is deterministic.
 * @param state Pointer to the GameState object.
 */
void game_spawn_tile(GameState *state);

/**
 * @brief slides tiles in the specified direction and merges adjacent equals.
 * @param state Pointer to the GameState object.
 * @param dir Direction to move (DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT).
 * @return 1 if the board changed, 0 if the move was invalid (no change).
 */
int game_slide(GameState *state, int dir);

/**
 * @brief Checks if the game is over (no empty slots and no adjacent merges).
 * @param state Pointer to the GameState object.
 * @return 1 if game is over, 0 otherwise.
 */
int game_check_over(const GameState *state);

#endif /* GAME_LOGIC_H */