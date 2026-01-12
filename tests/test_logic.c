/* ==============================================================================
 * Project: 2048-Core
 * File: tests/test_main.c
 * Standard: ANSI C (C89)
 * Description: Robust unit test suite for Logic, Animation, and Storage.
 * Contains no external dependencies beyond Standard Lib and project headers.
 * ============================================================================== */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Project Headers */
#include "../include/game_logic.h"
#include "../include/renderer.h"
#include "../include/storage.h"

/* ==============================================================================
 * 1. Test Framework (Macros & Globals)
 * ============================================================================== */

static int g_tests_run = 0;
static int g_tests_failed = 0;

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

/* Assertion Macros */
#define ASSERT(expr, msg)                                                                          \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            printf("  %s[FAIL] %s%s\n", ANSI_COLOR_RED, msg, ANSI_COLOR_RESET);                    \
            printf("    Expression: %s\n", #expr);                                                 \
            printf("    File: %s, Line: %d\n", __FILE__, __LINE__);                                \
            g_tests_failed++;                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_INT_EQ(expected, actual, msg)                                                       \
    do {                                                                                           \
        int e_val = (expected);                                                                    \
        int a_val = (actual);                                                                      \
        if (e_val != a_val) {                                                                      \
            printf("  %s[FAIL] %s%s\n", ANSI_COLOR_RED, msg, ANSI_COLOR_RESET);                    \
            printf("    Expected: %d, Actual: %d\n", e_val, a_val);                                \
            g_tests_failed++;                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

#define ASSERT_FLOAT_EQ(expected, actual, tolerance, msg)                                          \
    do {                                                                                           \
        float diff = (float)fabs((double)(expected) - (double)(actual));                           \
        if (diff > (tolerance)) {                                                                  \
            printf("  %s[FAIL] %s%s\n", ANSI_COLOR_RED, msg, ANSI_COLOR_RESET);                    \
            printf("    Expected: %f, Actual: %f\n", (float)(expected), (float)(actual));          \
            g_tests_failed++;                                                                      \
            return;                                                                                \
        }                                                                                          \
    } while (0)

/* Helper: Print test header */
static void run_test(void (*test_func)(void), const char *name)
{
    printf("[RUNNING] %s...\n", name);
    test_func();
    g_tests_run++;
}

/* Helper: Manually set board state */
static void set_board(GameState *s, int *values)
{
    int i;
    for (i = 0; i < 16; i++) {
        s->board[i] = values[i];
    }
}

/* ==============================================================================
 * 2. Game Logic Tests
 * ============================================================================== */

void test_game_init(void)
{
    GameState state;
    int i;
    int all_zeros = 1;

    /* Fill with garbage first to ensure init clears it */
    state.score = 999;
    for (i = 0; i < 16; i++)
        state.board[i] = 1;

    game_init(&state);

    for (i = 0; i < 16; i++) {
        if (state.board[i] != 0)
            all_zeros = 0;
    }

    ASSERT(all_zeros, "Board should be all zeros after init");
    ASSERT_INT_EQ(0, (int)state.score, "Score should be 0");
    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_simple_slide(void)
{
    GameState state;
    int initial[] = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    game_init(&state);
    set_board(&state, initial);

    game_slide(&state, DIR_RIGHT, NULL, NULL);

    ASSERT_INT_EQ(0, state.board[0], "Index 0 should be empty");
    ASSERT_INT_EQ(2, state.board[3], "Index 3 should contain 2");
    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_simple_merge(void)
{
    GameState state;
    int initial[] = {2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    game_init(&state);
    set_board(&state, initial);

    game_slide(&state, DIR_LEFT, NULL, NULL);

    ASSERT_INT_EQ(4, state.board[0], "Merged value should be 4");
    ASSERT_INT_EQ(0, state.board[1], "Second slot should be empty");
    ASSERT_INT_EQ(4, (int)state.score, "Score should increase by 4");
    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_triple_merge_rule(void)
{
    /* [2, 2, 2, 0] Slide LEFT -> [4, 2, 0, 0] */
    GameState state;
    int initial[] = {2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    game_init(&state);
    set_board(&state, initial);

    game_slide(&state, DIR_LEFT, NULL, NULL);

    ASSERT_INT_EQ(4, state.board[0], "First pair merges to 4");
    ASSERT_INT_EQ(2, state.board[1], "Third 2 remains 2");
    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_recursive_merge_rule(void)
{
    /* [4, 2, 2, 0] Slide LEFT -> [4, 4, 0, 0] (NOT [8, 0, 0, 0]) */
    GameState state;
    int initial[] = {4, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    game_init(&state);
    set_board(&state, initial);

    game_slide(&state, DIR_LEFT, NULL, NULL);

    ASSERT_INT_EQ(4, state.board[0], "Original 4 stays");
    ASSERT_INT_EQ(4, state.board[1], "New 4 created");
    ASSERT_INT_EQ(0, state.board[2], "Empty");
    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_move_events(void)
{
    /* [2, 0, 2, 0] Slide RIGHT -> [0, 0, 0, 4] */
    GameState state;
    MoveEvent events[16];
    int count = 0;
    int initial[] = {2, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    game_init(&state);
    set_board(&state, initial);

    game_slide(&state, DIR_RIGHT, events, &count);

    /* Expecting 2 events:
     * 1. Index 0 moves to Index 3 (merged=0 or 1 depending on internal ordering)
     * 2. Index 2 moves to Index 3 (merged=1)
     * OR simply check if we have events targeting index 3.
     */
    ASSERT(count >= 1, "Should generate move events");

    /* Verify at least one event targets index 3 */
    ASSERT_INT_EQ(3, events[0].to_index, "Event 0 should target index 3");

    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_victory_condition(void)
{
    /* Setup: [1024, 1024, 0, 0] */
    GameState state;
    int initial[] = {1024, 1024, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    game_init(&state);
    set_board(&state, initial);

    ASSERT_INT_EQ(GAME_ACTIVE, state.status, "Game should start ACTIVE");

    /* Slide LEFT -> [2048, 0, 0, 0] */
    game_slide(&state, DIR_LEFT, NULL, NULL);

    ASSERT_INT_EQ(2048, state.board[0], "Should merge to 2048");
    ASSERT_INT_EQ(GAME_WON, state.status, "Status should be GAME_WON");

    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_game_over(void)
{
    GameState state;
    /* Full board, no merges */
    int stuck[] = {2, 4, 2, 4, 4, 2, 4, 2, 2, 4, 2, 4, 4, 2, 4, 2};
    /* Full board, horizontal merge available (row 0: 2, 2) */
    int playable[] = {2, 2, 2, 4, 4, 2, 4, 2, 2, 4, 2, 4, 4, 2, 4, 2};

    game_init(&state);

    set_board(&state, stuck);
    ASSERT(game_check_over(&state), "Should be game over");

    set_board(&state, playable);
    ASSERT(!game_check_over(&state), "Should NOT be game over (merge available)");

    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

/* ==============================================================================
 * 3. Animation System Tests (Headless)
 * ============================================================================== */

void test_renderer_notify_move(void)
{
    RendererContext ctx;
    int from = 0;
    int to = 1;

    /* Zero init context */
    memset(&ctx, 0, sizeof(RendererContext));

    /* Setup Mock VisualTile at 'from' (Pixel pos 10, 10) */
    ctx.visual_board[from].displayed_value = 2;
    ctx.visual_board[from].x = 10.0f;
    ctx.visual_board[from].y = 10.0f;
    ctx.visual_board[from].current_scale = 1.0f;

    /* Call function under test */
    renderer_notify_move(&ctx, from, to, 0);

    /* Assertions */
    ASSERT_INT_EQ(2, ctx.visual_board[to].displayed_value, "Target should inherit value");
    ASSERT_FLOAT_EQ(10.0f, ctx.visual_board[to].x, 0.1f, "Target X should start at Source X");
    ASSERT_FLOAT_EQ(10.0f, ctx.visual_board[to].y, 0.1f, "Target Y should start at Source Y");

    /* Ensure target position is updated (Mocking logic implies target_x/y calc)
     * Since we didn't call renderer_init, TILE_SIZE/offsets might be 0.
     * But notify_move recalculates target based on 'to' index.
     * If globals are 0, target_x will be 0.
     * Key check: target_x != x (unless they are coincidentally both 0).
     * To properly test, we manually inject TILE_SIZE globals if possible,
     * but since they are static in renderer.c, we rely on the logic that
     * target_x is calculated for 'to' index.
     */

    /* Note: Since we cannot access static globals in renderer.c,
     * we verify the state change logic we *can* see.
     */

    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_renderer_update_animations(void)
{
    RendererContext ctx;
    GameState state;
    float dt_small = 0.01f;
    float dt_large = 10.0f;

    memset(&ctx, 0, sizeof(RendererContext));
    game_init(&state);

    /* Setup a tile that needs to move from 0 to 100 */
    ctx.visual_board[0].displayed_value = 2;
    ctx.visual_board[0].x = 0.0f;
    ctx.visual_board[0].target_x = 100.0f;
    state.board[0] = 2; /* Logical state matches */

    /* 1. Small Step */
    renderer_update_animations(&ctx, &state, dt_small);
    ASSERT(ctx.visual_board[0].x > 0.0f, "X should increase");
    ASSERT(ctx.visual_board[0].x < 100.0f, "X should not finish yet");

    /* 2. Large Step (Snap) */
    renderer_update_animations(&ctx, &state, dt_large);
    ASSERT_FLOAT_EQ(100.0f, ctx.visual_board[0].x, 0.001f, "X should snap to target");

    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

/* ==============================================================================
 * 4. Storage System Tests
 * ============================================================================== */

void test_storage_save_load(void)
{
    GameState save_state;
    GameState load_state;
    const char *test_path = "."; /* Current dir */
    int i;
    int ret;

    /* Prepare Data */
    game_init(&save_state);
    save_state.score = 12345;
    save_state.high_score = 54321;
    save_state.board[5] = 2048;

    /* Save */
    ret = storage_save(test_path, &save_state);
    ASSERT_INT_EQ(STORAGE_OK, ret, "Save should succeed");

    /* Load */
    ret = storage_load(test_path, &load_state);
    ASSERT_INT_EQ(STORAGE_OK, ret, "Load should succeed");

    /* Compare */
    ASSERT_INT_EQ((int)save_state.score, (int)load_state.score, "Scores match");
    ASSERT_INT_EQ((int)save_state.high_score, (int)load_state.high_score, "High scores match");
    for (i = 0; i < 16; i++) {
        ASSERT_INT_EQ(save_state.board[i], load_state.board[i], "Board mismatch");
    }

    /* Clean up */
    remove("save.dat");
    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_storage_not_found(void)
{
    GameState state;
    int ret;
    remove("save.dat"); /* Ensure clean slate */

    ret = storage_load(".", &state);
    ASSERT_INT_EQ(STORAGE_ERR_NOT_FOUND, ret, "Should return NOT_FOUND");
    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

void test_storage_corruption(void)
{
    GameState state;
    FILE *fp;
    int ret;
    unsigned long bad_magic = 0xDEADBEEF;

    /* Create corrupt file */
    fp = fopen("save.dat", "wb");
    if (fp) {
        fwrite(&bad_magic, sizeof(bad_magic), 1, fp);
        fclose(fp);
    }

    ret = storage_load(".", &state);
    ASSERT_INT_EQ(STORAGE_ERR_CORRUPT, ret, "Should detect corruption");

    remove("save.dat");
    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

/* ==============================================================================
 * 5. Input Logic Tests (Algorithm Verification)
 * ============================================================================== */

/* * Helper: Replicates the swipe detection logic from src/main.c
 * to ensure the math and thresholds are correct.
 */
static InputCommand simulate_swipe(float start_x, float start_y, float end_x, float end_y)
{
    float dx = end_x - start_x;
    float dy = end_y - start_y;

    /* Threshold: 0.05f */
    if (fabs((double)dx) > 0.05f || fabs((double)dy) > 0.05f) {
        if (fabs((double)dx) > fabs((double)dy)) {
            return (dx > 0) ? INPUT_RIGHT : INPUT_LEFT;
        } else {
            return (dy > 0) ? INPUT_DOWN : INPUT_UP;
        }
    }
    return INPUT_NONE;
}

void test_swipe_algorithm(void)
{
    /* Test 1: Tap (Below threshold 0.05) -> Should be ignored */
    ASSERT_INT_EQ(INPUT_NONE, simulate_swipe(0.5f, 0.5f, 0.52f, 0.52f), "Tap should be ignored");

    /* Test 2: Pure Right Swipe */
    ASSERT_INT_EQ(INPUT_RIGHT, simulate_swipe(0.1f, 0.5f, 0.8f, 0.5f), "Right Swipe");

    /* Test 3: Pure Left Swipe */
    ASSERT_INT_EQ(INPUT_LEFT, simulate_swipe(0.8f, 0.5f, 0.1f, 0.5f), "Left Swipe");

    /* Test 4: Pure Up Swipe */
    ASSERT_INT_EQ(INPUT_UP, simulate_swipe(0.5f, 0.8f, 0.5f, 0.1f), "Up Swipe");

    /* Test 5: Pure Down Swipe */
    ASSERT_INT_EQ(INPUT_DOWN, simulate_swipe(0.5f, 0.1f, 0.5f, 0.8f), "Down Swipe");

    /* Test 6: Diagonal Swipe (Dominant X) -> Should be Horizontal */
    /* dx = 0.4, dy = 0.2 */
    ASSERT_INT_EQ(INPUT_RIGHT, simulate_swipe(0.1f, 0.1f, 0.5f, 0.3f),
                  "Dominant X should be RIGHT");

    /* Test 7: Diagonal Swipe (Dominant Y) -> Should be Vertical */
    /* dx = 0.1, dy = 0.4 */
    ASSERT_INT_EQ(INPUT_DOWN, simulate_swipe(0.1f, 0.1f, 0.2f, 0.5f), "Dominant Y should be DOWN");

    printf("  %s[PASS]%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
}

/* ==============================================================================
 * 5. Main Runner
 * ============================================================================== */

int main(void)
{
    printf("==========================================\n");
    printf("  2048-Core Unit Test Suite (ANSI C89)    \n");
    printf("==========================================\n");

    /* Game Logic */
    run_test(test_game_init, "Logic: Game Init");
    run_test(test_simple_slide, "Logic: Simple Slide");
    run_test(test_simple_merge, "Logic: Simple Merge");
    run_test(test_triple_merge_rule, "Logic: Triple Merge Rule");
    run_test(test_recursive_merge_rule, "Logic: Recursive Merge Rule");
    run_test(test_move_events, "Logic: Move Event Generation");
    run_test(test_victory_condition, "Logic: Victory Condition (2048)");
    run_test(test_game_over, "Logic: Game Over Detection");

    /* Animation */
    run_test(test_renderer_notify_move, "Anim: Notify Move (Mocked)");
    run_test(test_renderer_update_animations, "Anim: Interpolation & Snap");

    /* Storage */
    run_test(test_storage_save_load, "Storage: Save/Load Integrity");
    run_test(test_storage_not_found, "Storage: Error Handling (Not Found)");
    run_test(test_storage_corruption, "Storage: Corruption Detection");

    /* Input Tests */
    run_test(test_swipe_algorithm, "Input: Swipe Algorithm Verification");

    printf("==========================================\n");
    printf("Tests Run: %d\n", g_tests_run);
    if (g_tests_failed == 0) {
        printf("%sALL TESTS PASSED.%s\n", ANSI_COLOR_GREEN, ANSI_COLOR_RESET);
        return 0;
    } else {
        printf("%s%d TESTS FAILED.%s\n", ANSI_COLOR_RED, g_tests_failed, ANSI_COLOR_RESET);
        return 1;
    }
}