/* ==============================================================================
 * Project: 2048-Core
 * File: src/renderer.c
 * Standard: ANSI C (C89)
 * Description: Procedural rendering with embedded font and official color palette.
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

/* Color definition struct */
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
static Color get_text_color(int value)
{
    Color c;
    if (value <= 4) {
        c.r = 119;
        c.g = 110;
        c.b = 101;
    } else {
        c.r = 249;
        c.g = 246;
        c.b = 242;
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

/* Draw a full number centered in the tile */
static void draw_number(SDL_Renderer *renderer, int value, int tile_x, int tile_y, int tile_w)
{
    int temp = value;
    int digits[10]; /* Store digits in reverse order */
    int count = 0;
    int i;
    int pixel_scale = 4; /* Scale of the 'pixels' of the font */
    int digit_width = 5 * pixel_scale;
    int digit_spacing = 2 * pixel_scale;
    int total_width;
    int start_draw_x;
    int start_draw_y;
    Color color = get_text_color(value);

    /* Extract digits */
    if (value == 0)
        return;

    while (temp > 0) {
        digits[count++] = temp % 10;
        temp /= 10;
    }

    /* Calculate centering */
    /* Reduce scale for large numbers to fit */
    if (value > 1000)
        pixel_scale = 3;
    if (value > 10000)
        pixel_scale = 2;

    digit_width = 5 * pixel_scale;
    digit_spacing = 1 * pixel_scale;

    total_width = (count * digit_width) + ((count - 1) * digit_spacing);
    start_draw_x = tile_x + (tile_w - total_width) / 2;
    start_draw_y = tile_y + (tile_w - (5 * pixel_scale)) / 2;

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

void renderer_draw(RendererContext *ctx, const GameState *state)
{
    int i, row, col, val;
    SDL_Rect rect;
    Color c;

    /* 1. Clear Screen (Background Color: #FAF8EF) */
    SDL_SetRenderDrawColor(ctx->renderer, 250, 248, 239, 255);
    SDL_RenderClear(ctx->renderer);

    /* 2. Draw Board Background Container (#BBADA0) */
    SDL_SetRenderDrawColor(ctx->renderer, 187, 173, 160, 255);
    rect.x = START_X;
    rect.y = START_Y;
    rect.w = BOARD_SIZE;
    rect.h = BOARD_SIZE;
    SDL_RenderFillRect(ctx->renderer, &rect);

    /* 3. Draw Tiles */
    for (i = 0; i < 16; i++) {
        row = i / 4;
        col = i % 4;
        val = state->board[i];

        rect.x = START_X + TILE_MARGIN + (col * (TILE_SIZE + TILE_MARGIN));
        rect.y = START_Y + TILE_MARGIN + (row * (TILE_SIZE + TILE_MARGIN));
        rect.w = TILE_SIZE;
        rect.h = TILE_SIZE;

        /* Draw Tile Background */
        c = get_tile_color(val);
        SDL_SetRenderDrawColor(ctx->renderer, c.r, c.g, c.b, 255);
        SDL_RenderFillRect(ctx->renderer, &rect);

        /* Draw Number */
        if (val > 0) {
            draw_number(ctx->renderer, val, rect.x, rect.y, TILE_SIZE);
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