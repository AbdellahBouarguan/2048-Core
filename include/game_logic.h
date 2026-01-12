#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

/* ==============================================================================
 * Project: 2048-Core
 * File: include/game_logic.h
 * Standard: ANSI C (C89)
 * Description: Core data structures, State Machine definitions, and API.
 * ============================================================================== */

/* Game Status Enumeration */
typedef enum { GAME_ACTIVE = 0, GAME_WON = 1, GAME_OVER = 2, GAME_ENDLESS = 3 } GameStatus;

/* Finite State Machine States */
typedef enum { STATE_MENU, STATE_PLAYING, STATE_GAMEOVER, STATE_VICTORY } AppState;

/* Movement Directions */
typedef enum { DIR_UP = 0, DIR_DOWN = 1, DIR_LEFT = 2, DIR_RIGHT = 3 } Direction;

/* Universal Input Abstraction */
typedef enum {
    INPUT_NONE,
    INPUT_UP,
    INPUT_DOWN,
    INPUT_LEFT,
    INPUT_RIGHT,
    INPUT_CONFIRM, /* Enter, Space, or A button */
    INPUT_RESET,   /* 'R' key */
    INPUT_EXIT     /* ESC or Back button */
} InputCommand;

/* NEW: Event structure for animation system */
typedef struct {
    int from_index;
    int to_index;
    int merged; /* boolean: 1 if this move resulted in a merge */
} MoveEvent;

/**
 * @struct GameState
 * @brief Holds the entire state of the 2048 game session.
 *
 * @var board 1D array representing 4x4 grid. Index = row * 4 + col.
 * @var score Current session score.
 * @var high_score Best score persisted across sessions.
 * @var status Current game logic status (Active, Won, Over).
 */
typedef struct {
    int board[16];
    unsigned long score;
    unsigned long high_score; /* NEW: Persistence */
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
 * @param events Buffer to store move events (can be NULL).
 * @param event_count Pointer to integer to write the number of events (can be NULL).
 * @return 1 if the board changed, 0 if the move was invalid (no change).
 */
int game_slide(GameState *state, int dir, MoveEvent *events, int *event_count);

/**
 * @brief Checks if the game is over (no empty slots and no adjacent merges).
 * @param state Pointer to the GameState object.
 * @return 1 if game is over, 0 otherwise.
 */
int game_check_over(const GameState *state);

#endif /* GAME_LOGIC_H */