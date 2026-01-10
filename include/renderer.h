#ifndef RENDERER_H
#define RENDERER_H

/* ==============================================================================
 * Project: 2048-Core
 * File: include/renderer.h
 * Standard: ANSI C (C89)
 * Description: Hardware-accelerated rendering interface (SDL2).
 * ============================================================================== */

#include "game_logic.h"
#include <SDL.h>

/**
 * @struct RendererContext
 * @brief Holds the SDL pointers required for rendering.
 */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *atlas;
} RendererContext;

/**
 * @brief Initializes the SDL video subsystem, window, and loads assets.
 * @param ctx Pointer to the context structure to initialize.
 * @return 0 on success, -1 on failure.
 */
int renderer_init(RendererContext *ctx);

/**
 * @brief Renders the current game state to the screen.
 * @param ctx Pointer to the initialized renderer context.
 * @param state Read-only pointer to the current game state.
 */
void renderer_draw(RendererContext *ctx, const GameState *state);

/**
 * @brief Cleans up all SDL resources.
 * @param ctx Pointer to the context to destroy.
 */
void renderer_cleanup(RendererContext *ctx);

#endif /* RENDERER_H */