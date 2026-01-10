/* ==============================================================================
 * Project: 2048-Core
 * File: tests/test_logic.c
 * Standard: ANSI C (C89)
 * Description: Minimal custom test runner.
 * ============================================================================== */

#include "../include/game_logic.h"
#include <stdio.h>
#include <stdlib.h>

/* ANSI Colors for output */
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"

static int g_tests_run = 0;
static int g_tests_failed = 0;

/* Helper: Manually set board state */
void set_board(GameState *s, int *values)
{
    int i;
    for (i = 0; i < 16; i++)
        s->board[i] = values[i];
    s->score = 0;
    s->status = GAME_ACTIVE;
}

/* Helper: Assertion */
void assert_eq(int expected, int actual, const char *msg)
{
    if (expected != actual) {
        printf("%s[FAIL]%s %s: Expected %d, got %d\n", RED, RESET, msg, expected, actual);
        g_tests_failed++;
    }
}

/* Helper: Array Assertion */
void assert_board(GameState *s, int *expected, const char *msg)
{
    int i;
    for (i = 0; i < 16; i++) {
        if (s->board[i] != expected[i]) {
            printf("%s[FAIL]%s %s at index %d: Expected %d, got %d\n", RED, RESET, msg, i,
                   expected[i], s->board[i]);
            g_tests_failed++;
            return;
        }
    }
    printf("%s[PASS]%s %s\n", GREEN, RESET, msg);
}

void test_basic_move(void)
{
    GameState state;
    int initial[] = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int expected[] = {0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    game_init(&state);
    set_board(&state, initial);

    game_slide(&state, DIR_RIGHT);

    assert_board(&state, expected, "Test 1: Basic Move Right");
    g_tests_run++;
}

void test_simple_merge(void)
{
    GameState state;
    int initial[] = {2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int expected[] = {0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    game_init(&state);
    set_board(&state, initial);

    game_slide(&state, DIR_RIGHT);

    assert_board(&state, expected, "Test 2: Merge Right");
    assert_eq(4, (int)state.score, "Test 2: Score Update");
    g_tests_run++;
}

void test_triple_merge(void)
{
    /* Rule: When 3 identical tiles are in a row, only the two leading in direction of travel merge
     */
    /* [2, 2, 2, 0] Slide RIGHT -> [0, 0, 2, 4] */
    GameState state;
    int initial[] = {2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int expected[] = {0, 0, 2, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    game_init(&state);
    set_board(&state, initial);

    game_slide(&state, DIR_RIGHT);

    assert_board(&state, expected, "Test 3: Triple Merge Edge Case");
    g_tests_run++;
}

void test_game_over(void)
{
    GameState state;
    int checkerboard[] = {2, 4, 2, 4, 4, 2, 4, 2, 2, 4, 2, 4, 4, 2, 4, 2};

    game_init(&state);
    set_board(&state, checkerboard);

    assert_eq(1, game_check_over(&state), "Test 4: Game Over Detection");
    g_tests_run++;
}

int main(void)
{
    printf("Running Logic Tests...\n");

    test_basic_move();
    test_simple_merge();
    test_triple_merge();
    test_game_over();

    printf("\nTests Run: %d\n", g_tests_run);
    printf("Tests Failed: %d\n", g_tests_failed);

    return g_tests_failed;
}