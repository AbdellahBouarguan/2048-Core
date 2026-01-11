#ifndef RENDERER_H
#define RENDERER_H

/* ==============================================================================
 * Project: 2048-Core
 * File: include/renderer.h
 * Standard: ANSI C (C89)
 * Description: Hardware-accelerated rendering interface with Animation support.
 * ============================================================================== */

#include "game_logic.h"
#include <SDL.h>

/**
 * @struct VisualTile
 * @brief Internal visual state for interpolation/animation.
 */
typedef struct {
    float current_scale;      /* 0.0f to 1.0f (animation progress) */
    float target_scale;       /* 0.0f (hidden) or 1.0f (shown) */
    int displayed_value;      /* The number currently being rendered */
    float x, y;               /* Current pixel position */
    float target_x, target_y; /* NEW: Target pixel position for sliding */
} VisualTile;

/**
 * @struct RendererContext
 * @brief Holds the SDL pointers and visual state required for rendering.
 */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *atlas;
    VisualTile visual_board[16]; /* Animation Layer */
} RendererContext;

/**
 * @brief Initializes the SDL video subsystem, window, and loads assets.
 */
int renderer_init(RendererContext *ctx);

/**
 * @brief Handles a move event by updating visual state mapping.
 * @param ctx Pointer to the renderer context.
 * @param from_index Source grid index.
 * @param to_index Destination grid index.
 * @param merged Boolean indicating if this is a merge event.
 */
void renderer_notify_move(RendererContext *ctx, int from_index, int to_index, int merged);

/**
 * @brief Updates the visual_board interpolation based on delta time (dt).
 * @param ctx Pointer to the renderer context.
 * @param state Pointer to the logical game state (target values).
 * @param dt Delta time in seconds since last frame.
 */
void renderer_update_animations(RendererContext *ctx, const GameState *state, float dt);

/**
 * @brief Renders the current game state to the screen based on AppState.
 * @param ctx Pointer to the initialized renderer context.
 * @param state Read-only pointer to the current game state.
 * @param app_state Current state of the FSM (Menu, Playing, GameOver).
 */
void renderer_draw(RendererContext *ctx, const GameState *state, AppState app_state);

/**
 * @brief Cleans up all SDL resources.
 */
void renderer_cleanup(RendererContext *ctx);

#endif /* RENDERER_H */