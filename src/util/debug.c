/*
 * Knight Engine 2D - Debug Utilities Implementation
 */

#include "util/debug.h"
#include "graphics/camera.h"
#include <math.h>

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
