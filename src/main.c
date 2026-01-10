/* ==============================================================================
 * Project: 2048-Core
 * File: src/main.c
 * Standard: ANSI C (C89)
 * Description: Application Entry Point, Event Loop, and Path Management.
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
    /* Android: Use SDL's internal storage mapping */
    const char *path = SDL_GetPrefPath("MyOrg", "2048Core");
    if (path) {
        strncpy(buffer, path, max_len);
        SDL_free((void *)path);
    } else {
        strcpy(buffer, ".");
    }
#elif defined(__linux__)
    /* Linux: Follow XDG Base Directory Specification (~/.local/share/2048-core/) */
    const char *home = getenv("HOME");
    char path[512];
    struct stat st; /* FIXED: Moved declaration to top to comply with C89 */

    if (home) {
        sprintf(path, "%s/.local/share/2048-core", home);

        /* Create directory if it doesn't exist (POSIX) */
        if (stat(path, &st) == -1) {
            mkdir(path, 0700);
            /* Also ensure parent exists? For simplicity, we assume ~/.local/share exists
             * or we fall back to local folder if mkdir fails.
             */
        }
        strncpy(buffer, path, max_len);
    } else {
        strcpy(buffer, ".");
    }
#else
    /* Fallback (Windows/Other): Local folder */
    strcpy(buffer, ".");
#endif
}

int main(int argc, char *argv[])
{
    /* C89 Variable Declarations */
    RendererContext ctx;
    GameState state;
    SDL_Event event;
    int running = 1;
    int moved = 0;
    int reset_flag = 0;
    int i;
    char save_path[512];
    Uint32 frame_start;
    int frame_time;

    /* 1. CLI Argument Parsing */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--reset") == 0) {
            reset_flag = 1;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("2048-Core v%s\n", APP_VERSION);
            return 0;
        }
    }

    /* 2. Initialization */
    srand((unsigned int)time(NULL));
    resolve_save_path(save_path, sizeof(save_path));
    printf("[System] Save Path: %s\n", save_path);

    if (renderer_init(&ctx) != 0) {
        fprintf(stderr, "[Fatal] Renderer failed to initialize.\n");
        return 1;
    }

    /* 3. Game State Loading */
    if (reset_flag) {
        printf("[Game] Reset requested. Starting fresh.\n");
        game_init(&state);
        game_spawn_tile(&state);
        game_spawn_tile(&state);
        /* Autosave the clean state immediately */
        storage_save(save_path, &state);
    } else {
        int load_result = storage_load(save_path, &state);
        if (load_result != STORAGE_OK) {
            printf("[Game] No valid save found (Code %d). Creating new game.\n", load_result);
            game_init(&state);
            game_spawn_tile(&state);
            game_spawn_tile(&state);
        } else {
            printf("[Game] Save loaded successfully.\n");
        }
    }

    /* Initial Render */
    renderer_draw(&ctx, &state);

    /* 4. The Game Loop */
    while (running) {
        frame_start = SDL_GetTicks();

        /* A. Input Polling */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                moved = 0;

                /* Don't allow moves if game is over, unless resetting */
                if (state.status != GAME_OVER) {
                    switch (event.key.keysym.sym) {
                    case SDLK_UP:
                    case SDLK_w:
                        moved = game_slide(&state, DIR_UP);
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        moved = game_slide(&state, DIR_DOWN);
                        break;
                    case SDLK_LEFT:
                    case SDLK_a:
                        moved = game_slide(&state, DIR_LEFT);
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        moved = game_slide(&state, DIR_RIGHT);
                        break;
                    }
                }

                /* Global Controls */
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = 0;
                } else if (event.key.keysym.sym == SDLK_r) {
                    /* Hard Reset */
                    game_init(&state);
                    game_spawn_tile(&state);
                    game_spawn_tile(&state);
                    storage_save(save_path, &state);
                    renderer_draw(&ctx, &state);
                    moved = 0; /* Prevent double render logic below */
                }

                /* B. Logic Update */
                if (moved) {
                    game_spawn_tile(&state);

                    /* Check Game Over logic */
                    if (game_check_over(&state)) {
                        state.status = GAME_OVER;
                        printf("[Game] Game Over! Final Score: %lu\n", state.score);
                    }

                    /* C. Autosave */
                    storage_save(save_path, &state);

                    /* D. Render */
                    renderer_draw(&ctx, &state);
                }
            }
        }

        /* E. Frame Rate Cap */
        frame_time = (int)(SDL_GetTicks() - frame_start);
        if (FRAME_DELAY > frame_time) {
            SDL_Delay((Uint32)(FRAME_DELAY - frame_time));
        }
    }

    /* 5. Shutdown */
    renderer_cleanup(&ctx);
    printf("[System] Shutdown complete.\n");

    return 0;
}