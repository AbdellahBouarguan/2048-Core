/* ==============================================================================
 * Project: 2048-Core
 * File: src/renderer.c
 * Standard: ANSI C (C89)
 * Description: Procedural rendering with fixed Score display.
 * ============================================================================== */

#include "renderer.h"
#include <stdio.h>

/* Constants for layout */
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_MARGIN 12

/* Calculated at runtime based on screen size */
static int TILE_SIZE = 0;
static int BOARD_SIZE = 0;
static int START_X = 0;
static int START_Y = 0;

/* ==============================================================================
 * 1. Embedded Assets: 5x5 Pixel Font
 * Each byte represents a row of pixels (bitmask).
 * ============================================================================== */
static const unsigned char FONT[10][5] = {
    {0x1F, 0x11, 0x11, 0x11, 0x1F}, /* 0 */
    {0x04, 0x0C, 0x04, 0x04, 0x0E}, /* 1 */
    {0x1F, 0x01, 0x1F, 0x10, 0x1F}, /* 2 */
    {0x1F, 0x01, 0x0F, 0x01, 0x1F}, /* 3 */
    {0x11, 0x11, 0x1F, 0x01, 0x01}, /* 4 */
    {0x1F, 0x10, 0x1F, 0x01, 0x1F}, /* 5 */
    {0x1F, 0x10, 0x1F, 0x11, 0x1F}, /* 6 */
    {0x1F, 0x01, 0x02, 0x04, 0x04}, /* 7 */
    {0x1F, 0x11, 0x1F, 0x11, 0x1F}, /* 8 */
    {0x1F, 0x11, 0x1F, 0x01, 0x1F}  /* 9 */
};

typedef struct {
    Uint8 r, g, b;
} Color;

/* ==============================================================================
 * 2. Helper Functions
 * ============================================================================== */

/* Get background color for a specific tile value */
static Color get_tile_color(int value)
{
    Color c;
    switch (value) {
    case 0:
        c.r = 205;
        c.g = 193;
        c.b = 180;
        break; /* Empty */
    case 2:
        c.r = 238;
        c.g = 228;
        c.b = 218;
        break;
    case 4:
        c.r = 237;
        c.g = 224;
        c.b = 200;
        break;
    case 8:
        c.r = 242;
        c.g = 177;
        c.b = 121;
        break;
    case 16:
        c.r = 245;
        c.g = 149;
        c.b = 99;
        break;
    case 32:
        c.r = 246;
        c.g = 124;
        c.b = 95;
        break;
    case 64:
        c.r = 246;
        c.g = 94;
        c.b = 59;
        break;
    case 128:
        c.r = 237;
        c.g = 207;
        c.b = 114;
        break;
    case 256:
        c.r = 237;
        c.g = 204;
        c.b = 97;
        break;
    case 512:
        c.r = 237;
        c.g = 200;
        c.b = 80;
        break;
    case 1024:
        c.r = 237;
        c.g = 197;
        c.b = 63;
        break;
    case 2048:
        c.r = 237;
        c.g = 194;
        c.b = 46;
        break;
    default:
        c.r = 60;
        c.g = 58;
        c.b = 50;
        break; /* Super high numbers */
    }
    return c;
}

/* Get text color (Dark for 2/4, White for others) */
static Color get_tile_text_color(int value)
{
    Color c;
    if (value <= 4) {
        c.r = 119;
        c.g = 110;
        c.b = 101; /* Dark Grey */
    } else {
        c.r = 249;
        c.g = 246;
        c.b = 242; /* White */
    }
    return c;
}

/* Draw a single digit using the embedded bitmask font */
static void draw_digit(SDL_Renderer *renderer, int digit, int x, int y, int size, Color color)
{
    int row, col;
    SDL_Rect pixel_rect;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

    for (row = 0; row < 5; row++) {
        for (col = 0; col < 5; col++) {
            /* Check if the bit at this column is set */
            /* We shift 0x10 (10000) to the right 'col' times */
            if (FONT[digit][row] & (0x10 >> col)) {
                pixel_rect.x = x + (col * size);
                pixel_rect.y = y + (row * size);
                pixel_rect.w = size;
                pixel_rect.h = size;
                SDL_RenderFillRect(renderer, &pixel_rect);
            }
        }
    }
}

/**
 * Draw a full number.
 * @param area_w: The width of the container (for centering).
 * @param size_override: If > 0, used for Tile scaling logic. If 0, used for Score logic.
 * @param color: Explicit color to draw the text.
 */
static void draw_number(SDL_Renderer *renderer, int value, int x, int y, int area_w,
                        int size_override, Color color)
{
    int temp = value;
    int digits[10]; /* Store digits in reverse order */
    int count = 0;
    int i;
    int pixel_scale;
    int digit_width;
    int digit_spacing;
    int total_width;
    int start_draw_x;
    int start_draw_y;

    /* Handle 0 explicitly so it draws */
    if (value == 0) {
        digits[count++] = 0;
    } else {
        while (temp > 0) {
            digits[count++] = temp % 10;
            temp /= 10;
        }
    }

    /* Determine Scale and Layout */
    if (size_override > 0) {
        /* Tile Logic: Scale down as numbers get larger to fit */
        pixel_scale = 4;
        if (value > 100)
            pixel_scale = 3;
        if (value > 1000)
            pixel_scale = 2;
        if (value > 10000)
            pixel_scale = 1;

        digit_width = 5 * pixel_scale;
        digit_spacing = 2 * pixel_scale;

        total_width = (count * digit_width) + ((count - 1) * digit_spacing);
        /* Center in Tile */
        start_draw_x = x + (area_w - total_width) / 2;
        start_draw_y = y + (area_w - (5 * pixel_scale)) / 2;
    } else {
        /* Score Logic: Fixed large size */
        pixel_scale = 4;
        digit_width = 5 * pixel_scale;
        digit_spacing = 2 * pixel_scale;

        total_width = (count * digit_width) + ((count - 1) * digit_spacing);
        /* Center Horizontally in Screen */
        start_draw_x = x + (area_w - total_width) / 2;
        start_draw_y = y; /* Top aligned */
    }

    /* Draw digits (iterating backwards because we extracted them backwards) */
    for (i = count - 1; i >= 0; i--) {
        draw_digit(renderer, digits[i], start_draw_x, start_draw_y, pixel_scale, color);
        start_draw_x += digit_width + digit_spacing;
    }
}

/* ==============================================================================
 * 3. Public API Implementation
 * ============================================================================== */

int renderer_init(RendererContext *ctx)
{
    /* Initialize SDL Video */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return -1;
    }

    /* Create Window */
    ctx->window =
        SDL_CreateWindow("2048-Core (Procedural)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (!ctx->window)
        return -1;

    /* Create Renderer */
    ctx->renderer =
        SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!ctx->renderer)
        return -1;

    /* Calculate Layout Dimensions */
    /* Use the smaller screen dimension to fit the square board */
    BOARD_SIZE = (SCREEN_WIDTH < SCREEN_HEIGHT ? SCREEN_WIDTH : SCREEN_HEIGHT) - 100;
    TILE_SIZE = (BOARD_SIZE - (5 * TILE_MARGIN)) / 4;
    START_X = (SCREEN_WIDTH - BOARD_SIZE) / 2;
    START_Y = (SCREEN_HEIGHT - BOARD_SIZE) / 2;

    ctx->atlas = NULL; /* Not using textures anymore */
    return 0;
}

/* Update signature to match header */
void renderer_draw(RendererContext *ctx, const GameState *state, AppState app_state)
{
    int i, row, col, val;
    SDL_Rect rect;
    Color c_bg;
    Color c_text;
    /* Fixed Colors */
    Color c_score = {119, 110, 101};

    /* Layout Variables */
    int score_y_pos = 20;
    int effective_start_y = START_Y + 30;

    /* 1. Clear Screen */
    SDL_SetRenderDrawColor(ctx->renderer, 250, 248, 239, 255);
    SDL_RenderClear(ctx->renderer);

    /* 2. State-Based Rendering */
    if (app_state == STATE_MENU) {
        /* Draw Title Screen */
        draw_number(ctx->renderer, 2048, 0, SCREEN_HEIGHT / 3, SCREEN_WIDTH, 0, c_score);
        /* (Optional: Add "Press Enter" text here if you have a font system) */
    } else {
        /* STATE_PLAYING or STATE_GAMEOVER */

        /* Draw Score */
        draw_number(ctx->renderer, (int)state->score, 0, score_y_pos, SCREEN_WIDTH, 0, c_score);

        /* Draw Board Background */
        SDL_SetRenderDrawColor(ctx->renderer, 187, 173, 160, 255);
        rect.x = START_X;
        rect.y = effective_start_y;
        rect.w = BOARD_SIZE;
        rect.h = BOARD_SIZE;
        SDL_RenderFillRect(ctx->renderer, &rect);

        /* Draw Tiles */
        for (i = 0; i < 16; i++) {
            row = i / 4;
            col = i % 4;
            val = state->board[i];

            rect.x = START_X + TILE_MARGIN + (col * (TILE_SIZE + TILE_MARGIN));
            rect.y = effective_start_y + TILE_MARGIN + (row * (TILE_SIZE + TILE_MARGIN));
            rect.w = TILE_SIZE;
            rect.h = TILE_SIZE;

            /* Draw Tile Background */
            c_bg = get_tile_color(val);
            SDL_SetRenderDrawColor(ctx->renderer, c_bg.r, c_bg.g, c_bg.b, 255);
            SDL_RenderFillRect(ctx->renderer, &rect);

            /* Draw Number */
            if (val > 0) {
                c_text = get_tile_text_color(val);
                draw_number(ctx->renderer, val, rect.x, rect.y, TILE_SIZE, TILE_SIZE, c_text);
            }
        }

        /* 3. Game Over Overlay */
        if (app_state == STATE_GAMEOVER) {
            SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ctx->renderer, 238, 228, 218, 180); /* Faded yellow overlay */
            rect.x = 0;
            rect.y = 0;
            rect.w = SCREEN_WIDTH;
            rect.h = SCREEN_HEIGHT;
            SDL_RenderFillRect(ctx->renderer, &rect);
            /* Draw "Game Over" text if possible, or just the overlay for now */
        }
    }

    SDL_RenderPresent(ctx->renderer);
}

void renderer_cleanup(RendererContext *ctx)
{
    if (ctx->renderer)
        SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window)
        SDL_DestroyWindow(ctx->window);
    SDL_Quit();
}