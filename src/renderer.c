/* ==============================================================================
 * Project: 2048-Core
 * File: src/renderer.c
 * Standard: ANSI C (C89)
 * Description: Procedural rendering with fixed Score display.
 * ============================================================================== */

#include "renderer.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* Constants for layout */
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_MARGIN 12

/* Calculated at runtime based on screen size */
static int TILE_SIZE = 0;
static int BOARD_SIZE = 0;
static int START_X = 0;
static int START_Y = 0;
static int EFFECTIVE_START_Y = 0;

/* ==============================================================================
 * 1a. Embedded Assets (Font)
 * ============================================================================== */
static const unsigned char FONT[10][5] = {
    {0x1F, 0x11, 0x11, 0x11, 0x1F}, {0x04, 0x0C, 0x04, 0x04, 0x0E}, {0x1F, 0x01, 0x1F, 0x10, 0x1F},
    {0x1F, 0x01, 0x0F, 0x01, 0x1F}, {0x11, 0x11, 0x1F, 0x01, 0x01}, {0x1F, 0x10, 0x1F, 0x01, 0x1F},
    {0x1F, 0x10, 0x1F, 0x11, 0x1F}, {0x1F, 0x01, 0x02, 0x04, 0x04}, {0x1F, 0x11, 0x1F, 0x11, 0x1F},
    {0x1F, 0x11, 0x1F, 0x01, 0x1F}};

/* ==============================================================================
 * 1b. Embedded Assets: 5x5 Pixel Alphabet (A-Z)
 * ============================================================================== */
static const unsigned char ALPHABET[26][5] = {
    {0x0E, 0x11, 0x1F, 0x11, 0x11}, /* A */
    {0x1E, 0x11, 0x1E, 0x11, 0x1E}, /* B */
    {0x0E, 0x11, 0x10, 0x11, 0x0E}, /* C */
    {0x1C, 0x12, 0x12, 0x12, 0x1C}, /* D */
    {0x1F, 0x10, 0x1E, 0x10, 0x1F}, /* E */
    {0x1F, 0x10, 0x1E, 0x10, 0x10}, /* F */
    {0x0E, 0x11, 0x10, 0x13, 0x0E}, /* G */
    {0x11, 0x11, 0x1F, 0x11, 0x11}, /* H */
    {0x0E, 0x04, 0x04, 0x04, 0x0E}, /* I */
    {0x1F, 0x02, 0x02, 0x12, 0x0C}, /* J */
    {0x11, 0x12, 0x1C, 0x12, 0x11}, /* K */
    {0x10, 0x10, 0x10, 0x10, 0x1F}, /* L */
    {0x11, 0x1B, 0x15, 0x11, 0x11}, /* M */
    {0x11, 0x19, 0x15, 0x13, 0x11}, /* N */
    {0x0E, 0x11, 0x11, 0x11, 0x0E}, /* O */
    {0x1E, 0x11, 0x1E, 0x10, 0x10}, /* P */
    {0x0E, 0x11, 0x11, 0x12, 0x0D}, /* Q */
    {0x1E, 0x11, 0x1E, 0x12, 0x11}, /* R */
    {0x0F, 0x10, 0x0E, 0x01, 0x1E}, /* S */
    {0x1F, 0x04, 0x04, 0x04, 0x04}, /* T */
    {0x11, 0x11, 0x11, 0x11, 0x0E}, /* U */
    {0x11, 0x11, 0x11, 0x0A, 0x04}, /* V */
    {0x11, 0x11, 0x15, 0x15, 0x0A}, /* W */
    {0x11, 0x0A, 0x04, 0x0A, 0x11}, /* X */
    {0x11, 0x0A, 0x04, 0x04, 0x04}, /* Y */
    {0x1F, 0x02, 0x04, 0x08, 0x1F}  /* Z */
};

/* Helpers */
typedef struct {
    Uint8 r, g, b;
} Color;

static float lerp(float start, float end, float t)
{
    return start + t * (end - start);
}

/* Helper to get grid pixel coordinates */
static void get_tile_pos(int index, float *x, float *y)
{
    int row = index / 4;
    int col = index % 4;
    *x = (float)(START_X + TILE_MARGIN + (col * (TILE_SIZE + TILE_MARGIN)));
    *y = (float)(EFFECTIVE_START_Y + TILE_MARGIN + (row * (TILE_SIZE + TILE_MARGIN)));
}

static Color get_tile_color(int value)
{
    /* Same color logic as before */
    Color c = {60, 58, 50};
    switch (value) {
    case 0:
        c.r = 205;
        c.g = 193;
        c.b = 180;
        break;
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
    int i, pixel_scale, digit_width, digit_spacing, total_width, start_draw_x, start_draw_y;

    if (value == 0)
        digits[count++] = 0;
    else
        while (temp > 0) {
            digits[count++] = temp % 10;
            temp /= 10;
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

/**
 * @brief Renders a string centered horizontally at the given Y position.
 * Supports A-Z (caps only), 0-9, and spaces.
 */
static void draw_string(SDL_Renderer *renderer, const char *text, int y, int screen_w, int scale,
                        Color color)
{
    int len = (int)strlen(text);
    int char_w = 5 * scale;
    int spacing = 2 * scale;
    int total_w = (len * char_w) + ((len - 1) * spacing);
    int start_x = (screen_w - total_w) / 2;
    int i, row, col;
    SDL_Rect rect;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

    for (i = 0; i < len; i++) {
        char c = text[i];
        const unsigned char *bitmap = NULL;

        if (c >= 'A' && c <= 'Z') {
            bitmap = ALPHABET[c - 'A'];
        } else if (c >= '0' && c <= '9') {
            bitmap = FONT[c - '0'];
        }

        if (bitmap) {
            for (row = 0; row < 5; row++) {
                for (col = 0; col < 5; col++) {
                    if (bitmap[row] & (0x10 >> col)) {
                        rect.x = start_x + (i * (char_w + spacing)) + (col * scale);
                        rect.y = y + (row * scale);
                        rect.w = scale;
                        rect.h = scale;
                        SDL_RenderFillRect(renderer, &rect);
                    }
                }
            }
        }
        /* Spaces simply advance the loop without drawing */
    }
}

/* ==============================================================================
 * 3. Public API Implementation
 * ============================================================================== */

int renderer_init(RendererContext *ctx)
{
    int i;
    memset(ctx, 0, sizeof(RendererContext));

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return -1;

    /* Create Window */
    ctx->window =
        SDL_CreateWindow("2048-Core (Animated)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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
    EFFECTIVE_START_Y = START_Y + 30;

    for (i = 0; i < 16; i++) {
        float tx, ty;
        get_tile_pos(i, &tx, &ty);
        ctx->visual_board[i].x = tx;
        ctx->visual_board[i].y = ty;
        ctx->visual_board[i].target_x = tx;
        ctx->visual_board[i].target_y = ty;
        ctx->visual_board[i].current_scale = 0.0f;
    }

    return 0;
}

void renderer_notify_move(RendererContext *ctx, int from_index, int to_index, int merged)
{
    (void)merged; /* Silence unused parameter warning */
    /* Copy visual state from 'from' to 'to' to initiate slide.
     * Even if merged is true, we visually slide the incoming tile to the target.
     */
    ctx->visual_board[to_index] = ctx->visual_board[from_index];

    /* Set the NEW target position based on destination index */
    get_tile_pos(to_index, &ctx->visual_board[to_index].target_x,
                 &ctx->visual_board[to_index].target_y);

    /* Keep the 'from' pixel coordinates as current x/y so it slides from there */
    /* (This is implicitly done by the struct copy above) */
}

void renderer_update_animations(RendererContext *ctx, const GameState *state, float dt)
{
    int i;
    float speed_pos = 20.0f * dt;   /* Sliding speed */
    float speed_scale = 15.0f * dt; /* Scaling speed */

    /* [FIX] Clamp interpolation factors to 1.0 to prevent overshooting */
    if (speed_pos > 1.0f)
        speed_pos = 1.0f;
    if (speed_scale > 1.0f)
        speed_scale = 1.0f;

    for (i = 0; i < 16; i++) {
        /* 1. Position Interpolation (Slide) */
        ctx->visual_board[i].x =
            lerp(ctx->visual_board[i].x, ctx->visual_board[i].target_x, speed_pos);
        ctx->visual_board[i].y =
            lerp(ctx->visual_board[i].y, ctx->visual_board[i].target_y, speed_pos);

        /* Snap position if close */
        if (fabs(ctx->visual_board[i].target_x - ctx->visual_board[i].x) < 0.5f)
            ctx->visual_board[i].x = ctx->visual_board[i].target_x;
        if (fabs(ctx->visual_board[i].target_y - ctx->visual_board[i].y) < 0.5f)
            ctx->visual_board[i].y = ctx->visual_board[i].target_y;

        /* 2. Scale and Value Management */
        if (state->board[i] > 0) {
            ctx->visual_board[i].target_scale = 1.0f;
            /* Update displayed value.
             * Note: In a polished version, this might wait until slide finishes for merges,
             * but immediate update is acceptable for this scope. */
            ctx->visual_board[i].displayed_value = state->board[i];
        } else {
            /* If the board logic says it's 0, we shrink it.
             * Unless it's currently sliding?
             * For the core logic, if it is 0, it means it's empty or moved away. */

            /* Check if this slot is the DESTINATION of a move.
             * If so, the board value is > 0 (handled above).
             * If it is 0, it is truly empty. */
            ctx->visual_board[i].target_scale = 0.0f;
        }

        /* Interpolate Scale */
        if (speed_scale > 1.0f)
            speed_scale = 1.0f;
        ctx->visual_board[i].current_scale = lerp(ctx->visual_board[i].current_scale,
                                                  ctx->visual_board[i].target_scale, speed_scale);

        if (fabs(ctx->visual_board[i].target_scale - ctx->visual_board[i].current_scale) < 0.01f) {
            ctx->visual_board[i].current_scale = ctx->visual_board[i].target_scale;
        }
    }
}

void renderer_draw(RendererContext *ctx, const GameState *state, AppState app_state)
{
    int i, size, offset, val, title_scale;
    float scale, pulse;
    SDL_Rect rect;
    Color c_bg;
    Color c_text;
    Color c_dark = {119, 110, 101};
    Color c_light = {249, 246, 242};
    Color c_overlay_text = {119, 110, 101};

    /* Layout Variables */
    int score_y_pos = 20;
    int effective_start_y = START_Y + 30;

    /* Draw Score */
    char score_buf[32];

    /* 1. Clear Screen */
    SDL_SetRenderDrawColor(ctx->renderer, 250, 248, 239, 255);
    SDL_RenderClear(ctx->renderer);

    /* 2. State-Based Rendering */
    if (app_state == STATE_MENU) {
        /* Pulse Animation */
        pulse = (float)sin((float)SDL_GetTicks() * 0.005f);
        /* Map sine [-1, 1] to scale [4, 5] roughly, or just simple scaling */
        title_scale = 10 + (int)(pulse * 1.0f); /* Base size 10, varies +/- 1 */

        /* Draw Title (using draw_number manual placement or simplified string) */
        /* Note: draw_string handles 0-9 so we can use it for "2048" */
        draw_string(ctx->renderer, "2048", SCREEN_HEIGHT / 3, SCREEN_WIDTH, title_scale, c_dark);

        /* Draw Instruction */
        draw_string(ctx->renderer, "PRESS ENTER", SCREEN_HEIGHT / 2 + 50, SCREEN_WIDTH, 3, c_dark);

    } else {
        /* STATE_PLAYING or STATE_GAMEOVER */

        sprintf(score_buf, "%lu", state->score);
        /* Using draw_number for the main score as before, or draw_string for consistency */
        draw_number(ctx->renderer, (int)state->score, 0, score_y_pos, SCREEN_WIDTH, 0, c_dark);

        /* Draw Board Background */
        SDL_SetRenderDrawColor(ctx->renderer, 187, 173, 160, 255);
        rect.x = START_X;
        rect.y = effective_start_y;
        rect.w = BOARD_SIZE;
        rect.h = BOARD_SIZE;
        SDL_RenderFillRect(ctx->renderer, &rect);

        /* Draw Tiles */
        for (i = 0; i < 16; i++) {
            /* Skip if effectively invisible */
            if (ctx->visual_board[i].current_scale < 0.01f)
                continue;

            val = ctx->visual_board[i].displayed_value;

            /* Apply Scale centered on current X/Y */
            scale = ctx->visual_board[i].current_scale;
            size = (int)((float)TILE_SIZE * scale);
            offset = (TILE_SIZE - size) / 2;

            rect.x = (int)ctx->visual_board[i].x + offset;
            rect.y = (int)ctx->visual_board[i].y + offset;
            rect.w = size;
            rect.h = size;

            /* Draw Tile Background */
            c_bg = get_tile_color(val);
            SDL_SetRenderDrawColor(ctx->renderer, c_bg.r, c_bg.g, c_bg.b, 255);
            SDL_RenderFillRect(ctx->renderer, &rect);

            /* Draw Number */
            if (val > 0) {
                c_text = get_tile_text_color(val);
                draw_number(ctx->renderer, val, rect.x, rect.y, size, size, c_text);
            }
        }

        /* 3. Overlays (Game Over / Victory) */
        if (app_state == STATE_GAMEOVER) {
            /* Semi-transparent background */
            SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ctx->renderer, 238, 228, 218, 190);
            rect.x = 0;
            rect.y = 0;
            rect.w = SCREEN_WIDTH;
            rect.h = SCREEN_HEIGHT;
            SDL_RenderFillRect(ctx->renderer, &rect);
            SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);

            draw_string(ctx->renderer, "GAME OVER", SCREEN_HEIGHT / 3, SCREEN_WIDTH, 5,
                        c_overlay_text);

            draw_string(ctx->renderer, "SCORE", SCREEN_HEIGHT / 2, SCREEN_WIDTH, 3, c_overlay_text);
            draw_string(ctx->renderer, score_buf, SCREEN_HEIGHT / 2 + 40, SCREEN_WIDTH, 4,
                        c_overlay_text);

            draw_string(ctx->renderer, "PRESS R TO RESTART", SCREEN_HEIGHT - 100, SCREEN_WIDTH, 2,
                        c_overlay_text);
        } else if (app_state == STATE_VICTORY) {
            /* Gold Semi-transparent Overlay */
            SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ctx->renderer, 237, 194, 46, 160);
            rect.x = 0;
            rect.y = 0;
            rect.w = SCREEN_WIDTH;
            rect.h = SCREEN_HEIGHT;
            SDL_RenderFillRect(ctx->renderer, &rect);
            SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);

            /* "YOU WIN" Text */
            draw_string(ctx->renderer, "YOU WIN", SCREEN_HEIGHT / 3, SCREEN_WIDTH, 5, c_light);

            /* Instruction Text */
            draw_string(ctx->renderer, "PRESS ENTER TO CONTINUE", SCREEN_HEIGHT / 2 + 50,
                        SCREEN_WIDTH, 2, c_light);
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