/* ==============================================================================
 * Project: 2048-Core
 * File: src/main.c
 * Standard: ANSI C (C89)
 * Description: Application Entry Point with FSM Architecture.
 * ============================================================================== */

#include <SDL.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include "game_logic.h"
#include "renderer.h"
#include "storage.h"

#define FPS_CAP 60
#define FRAME_DELAY (1000 / FPS_CAP)
#define APP_VERSION "1.0.0"

static float touch_start_x = 0.0f;
static float touch_start_y = 0.0f;
static int mouse_start_x = 0;
static int mouse_start_y = 0;

/* Helper: Resolves the save directory based on platform */
static void resolve_save_path(char *buffer, size_t max_len)
{
#ifdef __ANDROID__
    const char *path = SDL_GetPrefPath("MyOrg", "2048Core");
    if (path) {
        strncpy(buffer, path, max_len);
        SDL_free((void *)path);
    } else {
        strcpy(buffer, ".");
    }
#elif defined(__linux__)
    const char *home = getenv("HOME");
    char path[512];
    struct stat st;
    if (home) {
        sprintf(path, "%s/.local/share/2048-core", home);
        if (stat(path, &st) == -1)
            mkdir(path, 0700);
        strncpy(buffer, path, max_len);
    } else {
        strcpy(buffer, ".");
    }
#else
    strcpy(buffer, ".");
#endif
}

/* [NEW] Input Queue Implementation */
#define INPUT_QUEUE_SIZE 4

typedef struct {
    InputCommand buffer[INPUT_QUEUE_SIZE];
    int head;
    int tail;
    int count;
} InputQueue;

static void queue_init(InputQueue *q)
{
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

static void queue_push(InputQueue *q, InputCommand cmd)
{
    if (q->count < INPUT_QUEUE_SIZE) {
        q->buffer[q->tail] = cmd;
        q->tail = (q->tail + 1) % INPUT_QUEUE_SIZE;
        q->count++;
    }
}

static InputCommand queue_pop(InputQueue *q)
{
    InputCommand cmd = INPUT_NONE;
    if (q->count > 0) {
        cmd = q->buffer[q->head];
        q->head = (q->head + 1) % INPUT_QUEUE_SIZE;
        q->count--;
    }
    return cmd;
}

static int queue_is_empty(InputQueue *q)
{
    return (q->count == 0);
}

/* Input Abstraction Layer */
static void handle_input(SDL_Event *e, InputQueue *q)
{
    InputCommand cmd = INPUT_NONE;
    if (e->type == SDL_QUIT) {
        cmd = INPUT_EXIT;
    } else if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            cmd = INPUT_UP;
            break;
        case SDLK_DOWN:
        case SDLK_s:
            cmd = INPUT_DOWN;
            break;
        case SDLK_LEFT:
        case SDLK_a:
            cmd = INPUT_LEFT;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            cmd = INPUT_RIGHT;
            break;
        case SDLK_RETURN:
        case SDLK_SPACE:
            cmd = INPUT_CONFIRM;
            break;
        case SDLK_r:
            cmd = INPUT_RESET;
            break;
        case SDLK_ESCAPE:
            cmd = INPUT_EXIT;
            break;
        default:
            break;
        }
    }
    /* [NEW] Touch Swipe Handling */
    else if (e->type == SDL_FINGERDOWN) {
        touch_start_x = e->tfinger.x;
        touch_start_y = e->tfinger.y;
    } else if (e->type == SDL_FINGERUP) {
        float dx = e->tfinger.x - touch_start_x;
        float dy = e->tfinger.y - touch_start_y;
        /* Threshold: 0.05 (5% of screen) to ignore accidental taps */
        if (fabs(dx) > 0.05f || fabs(dy) > 0.05f) {
            if (fabs(dx) > fabs(dy)) {
                cmd = (dx > 0) ? INPUT_RIGHT : INPUT_LEFT;
            } else {
                cmd = (dy > 0) ? INPUT_DOWN : INPUT_UP;
            }
        }
    } /* [NEW] Mouse Swipe Handling (for testing/PC) */
    else if (e->type == SDL_MOUSEBUTTONDOWN) {
        mouse_start_x = e->button.x;
        mouse_start_y = e->button.y;
    } else if (e->type == SDL_MOUSEBUTTONUP) {
        int dx = e->button.x - mouse_start_x;
        int dy = e->button.y - mouse_start_y;
        /* Threshold: 50 pixels */
        if (abs(dx) > 50 || abs(dy) > 50) {
            if (abs(dx) > abs(dy)) {
                cmd = (dx > 0) ? INPUT_RIGHT : INPUT_LEFT;
            } else {
                cmd = (dy > 0) ? INPUT_DOWN : INPUT_UP;
            }
        }
    }

    if (cmd != INPUT_NONE) {
        /* Force EXIT to be immediate/handled by caller logic or pushed to front?
         * Simple push is fine, but checking for EXIT in the event loop is safer. */
        queue_push(q, cmd);
    }
}

int main(int argc, char *argv[])
{
    /* Variables */
    RendererContext ctx;
    GameState state;
    SDL_Event event;
    int running = 1;
    int i;
    char save_path[512];
    Uint32 frame_start;
    int frame_time;

    /* Time management */
    Uint32 last_time = 0;
    Uint32 current_time = 0;
    float dt = 0.0f;

    /* FSM State */
    AppState app_state = STATE_MENU;
    /* InputCommand cmd = INPUT_NONE; */
    InputCommand frame_cmd = INPUT_NONE;
    Uint32 game_over_timer = 0;
    int reset_flag = 0;

    /* NEW: Event Buffer */
    MoveEvent move_events[32]; /* Max 16 moves + merges, 32 is safe */
    int move_event_count = 0;

    InputQueue input_queue;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--reset") == 0)
            reset_flag = 1;
        else if (strcmp(argv[i], "--version") == 0) {
            printf("2048-Core v%s\n", APP_VERSION);
            return 0;
        }
    }

    /* Initialization */
    srand((unsigned int)time(NULL));
    resolve_save_path(save_path, sizeof(save_path));

    if (renderer_init(&ctx) != 0) {
        fprintf(stderr, "[Fatal] Renderer failed to initialize.\n");
        return 1;
    }

    /* Game State Loading */
    if (reset_flag) {
        game_init(&state);
        game_spawn_tile(&state);
        game_spawn_tile(&state);
        storage_save(save_path, &state);
    } else {
        if (storage_load(save_path, &state) != STORAGE_OK) {
            game_init(&state);
            game_spawn_tile(&state);
            game_spawn_tile(&state);
        }
    }

    /* Initialize timing */
    last_time = SDL_GetTicks();

    queue_init(&input_queue);

    /* The Game Loop (FSM Refactor) */
    while (running) {
        frame_start = SDL_GetTicks();
        current_time = frame_start;
        dt = (float)(current_time - last_time) / 1000.0f;
        last_time = current_time;

        frame_cmd = INPUT_NONE;

        /* Input Polling */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
            else
                handle_input(&event, &input_queue);
        }

        /* Pop one command per frame if available */
        frame_cmd = INPUT_NONE;
        if (!queue_is_empty(&input_queue)) {
            frame_cmd = queue_pop(&input_queue);
            if (frame_cmd == INPUT_EXIT)
                running = 0;
        }

        /* State Machine Logic */
        switch (app_state) {
        case STATE_MENU:
            if (frame_cmd == INPUT_CONFIRM) {
                int has_tiles = 0;
                for (i = 0; i < 16; i++)
                    if (state.board[i] != 0)
                        has_tiles = 1;
                if (!has_tiles) {
                    game_init(&state);
                    game_spawn_tile(&state);
                    game_spawn_tile(&state);
                }
                app_state = STATE_PLAYING;
            }
            break;

        case STATE_PLAYING:
            /* Handle Move Inputs */
            {
                int moved = 0;
                move_event_count = 0;

                if (frame_cmd == INPUT_UP)
                    moved = game_slide(&state, DIR_UP, move_events, &move_event_count);
                else if (frame_cmd == INPUT_DOWN)
                    moved = game_slide(&state, DIR_DOWN, move_events, &move_event_count);
                else if (frame_cmd == INPUT_LEFT)
                    moved = game_slide(&state, DIR_LEFT, move_events, &move_event_count);
                else if (frame_cmd == INPUT_RIGHT)
                    moved = game_slide(&state, DIR_RIGHT, move_events, &move_event_count);

                if (moved) {
                    /* Process Animation Events */
                    for (i = 0; i < move_event_count; i++) {
                        renderer_notify_move(&ctx, move_events[i].from_index,
                                             move_events[i].to_index, move_events[i].merged);
                    }

                    game_spawn_tile(&state);
                    storage_save(save_path, &state);
                    game_over_timer = 0;
                }

                /* [NEW] Victory Transition */
                if (state.status == GAME_WON) {
                    app_state = STATE_VICTORY;
                }

                if (game_check_over(&state)) {
                    if (game_over_timer == 0)
                        game_over_timer = SDL_GetTicks();
                    if (SDL_GetTicks() - game_over_timer > 1000) {
                        state.status = GAME_OVER;
                        app_state = STATE_GAMEOVER;
                    }
                } else {
                    game_over_timer = 0;
                }

                if (frame_cmd == INPUT_RESET) {
                    game_init(&state);
                    game_spawn_tile(&state);
                    game_spawn_tile(&state);
                    storage_save(save_path, &state);
                    game_over_timer = 0;
                }
            }
            break;

        /* [NEW] Victory State */
        case STATE_VICTORY:
            if (frame_cmd == INPUT_CONFIRM) {
                state.status = GAME_ENDLESS; /* Set to Endless to prevent loop */
                app_state = STATE_PLAYING;
            }
            break;

        case STATE_GAMEOVER:
            if (frame_cmd == INPUT_RESET || frame_cmd == INPUT_CONFIRM) {
                game_init(&state);
                game_spawn_tile(&state);
                game_spawn_tile(&state);
                storage_save(save_path, &state);
                app_state = STATE_PLAYING;
            }
            break;
        }

        /* Render */
        renderer_update_animations(&ctx, &state, dt); /* NEW: Update Animations */
        renderer_draw(&ctx, &state, app_state);

        /* Frame Rate Cap */
        frame_time = (int)(SDL_GetTicks() - frame_start);
        if (FRAME_DELAY > frame_time) {
            SDL_Delay((Uint32)(FRAME_DELAY - frame_time));
        }
    }

    renderer_cleanup(&ctx);
    return 0;
}