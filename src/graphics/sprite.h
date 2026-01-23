/*
 * Knight Engine 2D - Sprite System
 *
 * Represents any renderable game object with position, velocity,
 * dimensions, and texture.
 */

#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

/* Forward declaration */
typedef struct camera_t camera_t;

/*
 * Sprite structure - represents any renderable game object
 * Combines position, velocity, dimensions, and texture into one unit.
 * Using floats for position/velocity enables smooth sub-pixel movement.
 */
typedef struct {
    float x;
    float y;
    float vel_x;
    float vel_y;
    int width;
    int height;
    int z_index;          /* Render order: 0 = background, 50 = entities, 100 = UI */
    double angle;         /* Rotation in degrees (clockwise) */
    SDL_RendererFlip flip; /* SDL_FLIP_NONE, SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL */
    SDL_Texture *texture;
    /* Debug visualization */
    bool show_debug_bounds;  /* Draw bounding box when debug mode is on */
    Uint8 debug_r, debug_g, debug_b;  /* Debug border color */
} sprite_t;

/*
 * Render a sprite to the screen
 * Call this for each sprite during the render phase.
 *
 * camera:   Camera for world-to-screen coordinate conversion.
 * src_rect: Optional source rectangle for sprite sheets.
 *           Pass NULL to render the entire texture.
 *           When non-NULL, specifies which portion of the texture to render.
 */
void sprite_render(SDL_Renderer *renderer, const sprite_t *sprite,
                   const camera_t *camera, const SDL_Rect *src_rect);

/*
 * Render a sprite with extended options (rotation, flip, color modulation)
 *
 * camera:   Camera for world-to-screen coordinate conversion.
 * src_rect:  Optional source rectangle for sprite sheets (NULL = full texture)
 * angle:     Rotation in degrees (clockwise)
 * center:    Point to rotate around (NULL = center of sprite)
 * flip:      SDL_FLIP_NONE, SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL, or combined
 * r, g, b:   Color modulation (255 = no change, lower = tint toward that color)
 */
void sprite_render_ex(SDL_Renderer *renderer, const sprite_t *sprite,
                      const camera_t *camera, const SDL_Rect *src_rect,
                      double angle, const SDL_Point *center,
                      SDL_RendererFlip flip, Uint8 r, Uint8 g, Uint8 b);
