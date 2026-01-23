/*
 * Knight Engine 2D - Debug Utilities
 *
 * Visual debugging tools for rendering debug information.
 */

#pragma once

#include <SDL2/SDL.h>

/* Forward declaration */
typedef struct camera_t camera_t;

/*
 * Draw a colored rectangle outline (for collision boxes, debug bounds, etc.)
 * Uses world coordinates - converts to screen space using camera.
 */
void debug_draw_rect(SDL_Renderer *renderer, const camera_t *camera,
                     float world_x, float world_y, int width, int height,
                     Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*
 * Draw a filled colored rectangle (for debug visualization)
 * Uses world coordinates - converts to screen space using camera.
 */
void debug_fill_rect(SDL_Renderer *renderer, const camera_t *camera,
                     float world_x, float world_y, int width, int height,
                     Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/*
 * Draw a rotated rectangle outline (for debug bounds on rotated sprites)
 * Angle is in degrees (clockwise), rotation is around the center of the rect.
 */
void debug_draw_rect_rotated(SDL_Renderer *renderer, const camera_t *camera,
                             float world_x, float world_y, int width, int height,
                             double angle, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
