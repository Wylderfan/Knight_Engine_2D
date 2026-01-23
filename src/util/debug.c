/*
 * Knight Engine 2D - Debug Utilities Implementation
 */

#include "util/debug.h"
#include "core/config.h"
#include "core/game_state.h"
#include "graphics/camera.h"
#include "graphics/renderer.h"
#include "graphics/texture.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void debug_draw_rect(SDL_Renderer *renderer, const camera_t *camera,
                     float world_x, float world_y, int width, int height,
                     Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int screen_x, screen_y;
    world_to_screen(camera, world_x, world_y, &screen_x, &screen_y);

    SDL_Rect rect = { screen_x, screen_y, width, height };

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawRect(renderer, &rect);
}

void debug_fill_rect(SDL_Renderer *renderer, const camera_t *camera,
                     float world_x, float world_y, int width, int height,
                     Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int screen_x, screen_y;
    world_to_screen(camera, world_x, world_y, &screen_x, &screen_y);

    SDL_Rect rect = { screen_x, screen_y, width, height };

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
}

void debug_draw_rect_rotated(SDL_Renderer *renderer, const camera_t *camera,
                             float world_x, float world_y, int width, int height,
                             double angle, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int screen_x, screen_y;
    world_to_screen(camera, world_x, world_y, &screen_x, &screen_y);

    /* Center of the rectangle */
    float cx = screen_x + width / 2.0f;
    float cy = screen_y + height / 2.0f;

    /* Convert angle to radians (SDL uses clockwise degrees) */
    double rad = angle * M_PI / 180.0;
    float cos_a = (float)cos(rad);
    float sin_a = (float)sin(rad);

    /* Four corners relative to center (before rotation) */
    float hw = width / 2.0f;
    float hh = height / 2.0f;

    /* Rotate each corner around center */
    SDL_Point corners[4];
    float offsets[4][2] = {
        { -hw, -hh },  /* Top-left */
        {  hw, -hh },  /* Top-right */
        {  hw,  hh },  /* Bottom-right */
        { -hw,  hh }   /* Bottom-left */
    };

    for (int i = 0; i < 4; i++) {
        float rx = offsets[i][0] * cos_a - offsets[i][1] * sin_a;
        float ry = offsets[i][0] * sin_a + offsets[i][1] * cos_a;
        corners[i].x = (int)(cx + rx);
        corners[i].y = (int)(cy + ry);
    }

    /* Draw lines between corners */
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawLine(renderer, corners[0].x, corners[0].y, corners[1].x, corners[1].y);
    SDL_RenderDrawLine(renderer, corners[1].x, corners[1].y, corners[2].x, corners[2].y);
    SDL_RenderDrawLine(renderer, corners[2].x, corners[2].y, corners[3].x, corners[3].y);
    SDL_RenderDrawLine(renderer, corners[3].x, corners[3].y, corners[0].x, corners[0].y);
}

void debug_stress_test_toggle(game_state_t *game) {
    if (game->stress_test_active) {
        /* Despawn: destroy textures and reset count */
        for (int i = game->stress_test_base_index; i < game->sprite_count; i++) {
            if (game->sprites[i].texture) {
                SDL_DestroyTexture(game->sprites[i].texture);
            }
        }
        game->sprite_count = game->stress_test_base_index;
        game->stress_test_active = false;
        printf("[STRESS_TEST] Disabled - %d sprites now active\n", game->sprite_count);
    } else {
        /* Spawn stress test sprites */
        game->stress_test_base_index = game->sprite_count;
        int spawned = 0;
        for (int i = 0; i < STRESS_TEST_SPRITE_COUNT && game->sprite_count < SPRITE_MAX_COUNT; i++) {
            sprite_t *spr = &game->sprites[game->sprite_count++];
            spr->texture = texture_create_colored(renderer_get_sdl(&game->renderer), 32, 32,
                (Uint8)(rand() % 256), (Uint8)(rand() % 256), (Uint8)(rand() % 256));
            /* Scatter across a larger world area */
            spr->x = (float)(rand() % (WINDOW_WIDTH * 2)) - WINDOW_WIDTH / 2;
            spr->y = (float)(rand() % (WINDOW_HEIGHT * 2)) - WINDOW_HEIGHT / 2;
            spr->vel_x = (float)(rand() % 100 - 50);
            spr->vel_y = (float)(rand() % 100 - 50);
            spr->width = 32;
            spr->height = 32;
            spr->z_index = 30 + (rand() % 20);
            spr->angle = (double)(rand() % 360);
            spr->flip = SDL_FLIP_NONE;
            spr->show_debug_bounds = false;
            spawned++;
        }
        game->stress_test_active = true;
        printf("[STRESS_TEST] Enabled - spawned %d sprites (%d total)\n",
               spawned, game->sprite_count);
    }
}
