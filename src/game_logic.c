/* ==============================================================================
 * Project: 2048-Core
 * File: src/game_logic.c
 * Standard: ANSI C (C89)
 * ============================================================================== */

#include "game_logic.h"
#include <stdlib.h>
#include <string.h>

/* Internal Helper: Processes a single line of 4 integers (standard 2048 rule).
 * Returns score increase.
 * The 'line' array is modified in place.
 * 'line[0]' is the 'farthest' tile (the wall we are sliding towards).
 */
static unsigned long merge_line(int *line)
{
    int i, target;
    unsigned long score_inc = 0;

    /* Step 1: Compress (Move non-zeros to front) */
    target = 0;
    for (i = 0; i < 4; i++) {
        if (line[i] != 0) {
            line[target] = line[i];
            if (target != i) {
                line[i] = 0;
            }
            target++;
        }
    }

    /* Step 2: Merge adjacent equals */
    for (i = 0; i < 3; i++) {
        if (line[i] != 0 && line[i] == line[i + 1]) {
            line[i] *= 2;
            line[i + 1] = 0;
            score_inc += (unsigned long)line[i];
        }
    }

    /* Step 3: Compress again */
    target = 0;
    for (i = 0; i < 4; i++) {
        if (line[i] != 0) {
            line[target] = line[i];
            if (target != i) {
                line[i] = 0;
            }
            target++;
        }
    }

    return score_inc;
}

void game_init(GameState *state)
{
    int i;
    /* Reset gameplay variables */
    state->score = 0;
    state->status = GAME_ACTIVE;

    /* Clear board */
    for (i = 0; i < 16; i++) {
        state->board[i] = 0;
    }

    /* Note: state->high_score is explicitly NOT reset here to preserve it
     * across game sessions or restarts. It must be initialized to 0
     * by the caller (storage loader) on the very first run.
     */
}

void game_spawn_tile(GameState *state)
{
    int empty_indices[16];
    int count = 0;
    int i;
    int rand_index;
    int val;

    /* Find all empty slots */
    for (i = 0; i < 16; i++) {
        if (state->board[i] == 0) {
            empty_indices[count++] = i;
        }
    }

    if (count == 0)
        return;

#ifdef TEST_MODE
    /* Deterministic for Testing */
    rand_index = empty_indices[0];
    val = 2;
#else
    rand_index = empty_indices[rand() % count];
    val = (rand() % 10 < 9) ? 2 : 4; /* 90% chance of 2 */
#endif

    state->board[rand_index] = val;
}

int game_slide(GameState *state, int dir)
{
    int i, j;
    int line[4];
    int original_board[16];
    int changed = 0;
    unsigned long turn_score = 0;

    /* Backup board to detect changes */
    for (i = 0; i < 16; i++)
        original_board[i] = state->board[i];

    /* Process 4 rows or columns */
    for (i = 0; i < 4; i++) {
        /* Extract line based on direction */
        for (j = 0; j < 4; j++) {
            int idx = 0;
            if (dir == DIR_LEFT)
                idx = i * 4 + j;
            else if (dir == DIR_RIGHT)
                idx = i * 4 + (3 - j);
            else if (dir == DIR_UP)
                idx = j * 4 + i;
            else if (dir == DIR_DOWN)
                idx = (3 - j) * 4 + i;

            line[j] = state->board[idx];
        }

        /* Process logic */
        turn_score += merge_line(line);

        /* Write back */
        for (j = 0; j < 4; j++) {
            int idx = 0;
            if (dir == DIR_LEFT)
                idx = i * 4 + j;
            else if (dir == DIR_RIGHT)
                idx = i * 4 + (3 - j);
            else if (dir == DIR_UP)
                idx = j * 4 + i;
            else if (dir == DIR_DOWN)
                idx = (3 - j) * 4 + i;

            state->board[idx] = line[j];
        }
    }

    /* Compare with original */
    for (i = 0; i < 16; i++) {
        if (state->board[i] != original_board[i]) {
            changed = 1;
            break;
        }
    }

    if (changed) {
        state->score += turn_score;

        /* Update High Score immediately */
        if (state->score > state->high_score) {
            state->high_score = state->score;
        }

        /* Check for 2048 tile to set Win state */
        for (i = 0; i < 16; i++) {
            if (state->board[i] == 2048 && state->status != GAME_WON) {
                state->status = GAME_WON;
            }
        }
    }

    return changed;
}

int game_check_over(const GameState *state)
{
    int x, y, current;

    /* 1. Check for empty slots */
    for (x = 0; x < 16; x++) {
        if (state->board[x] == 0)
            return 0;
    }

    /* 2. Check for possible merges (Horizontal & Vertical) */
    for (y = 0; y < 4; y++) {
        for (x = 0; x < 4; x++) {
            current = state->board[y * 4 + x];
            /* Check Right */
            if (x < 3 && state->board[y * 4 + (x + 1)] == current)
                return 0;
            /* Check Down */
            if (y < 3 && state->board[(y + 1) * 4 + x] == current)
                return 0;
        }
    }

    return 1; /* Game Over */
}