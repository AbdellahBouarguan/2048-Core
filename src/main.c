/* ==============================================================================
 * Project: 2048-Core
 * File: src/main.c
 * Standard: ANSI C (C89)
 * Description: Application Entry Point with FSM Architecture.
 * ============================================================================== */

#include <SDL.h>
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

/* Input Abstraction Layer */
static InputCommand handle_input(SDL_Event *e)
{
    if (e->type == SDL_QUIT)
        return INPUT_EXIT;
    if (e->type == SDL_KEYDOWN) {
        switch (e->key.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            return INPUT_UP;
        case SDLK_DOWN:
        case SDLK_s:
            return INPUT_DOWN;
        case SDLK_LEFT:
        case SDLK_a:
            return INPUT_LEFT;
        case SDLK_RIGHT:
        case SDLK_d:
            return INPUT_RIGHT;
        case SDLK_RETURN:
        case SDLK_SPACE:
            return INPUT_CONFIRM;
        case SDLK_r:
            return INPUT_RESET;
        case SDLK_ESCAPE:
            return INPUT_EXIT;
        default:
            return INPUT_NONE;
        }
    }
    return INPUT_NONE;
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
    InputCommand cmd = INPUT_NONE;
    InputCommand frame_cmd = INPUT_NONE;
    Uint32 game_over_timer = 0;
    int reset_flag = 0;

    /* NEW: Event Buffer */
    MoveEvent move_events[32]; /* Max 16 moves + merges, 32 is safe */
    int move_event_count = 0;

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

    /* The Game Loop (FSM Refactor) */
    while (running) {
        frame_start = SDL_GetTicks();
        current_time = frame_start;
        dt = (float)(current_time - last_time) / 1000.0f;
        last_time = current_time;

        frame_cmd = INPUT_NONE;

        /* Input Polling */
        while (SDL_PollEvent(&event)) {
            cmd = handle_input(&event);
            if (cmd == INPUT_EXIT)
                running = 0;
            else if (cmd != INPUT_NONE)
                frame_cmd = cmd;
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