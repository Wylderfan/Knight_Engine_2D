/*
 * Knight Engine 2D - Renderer System
 *
 * Wraps SDL rendering operations for easier management.
 */

#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>

/*
 * Renderer context - wraps SDL window and renderer
 */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int width;
    int height;
} renderer_t;

/*
 * Initialize the rendering system
 * Creates window and hardware-accelerated renderer with VSYNC.
 * Returns true on success, false on failure.
 */
bool renderer_init(renderer_t *rend, const char *title, int width, int height);

/*
 * Clean up the rendering system
 * Destroys renderer and window.
 */
void renderer_cleanup(renderer_t *rend);

/*
 * Clear the screen with a solid color
 */
void renderer_clear(renderer_t *rend, Uint8 r, Uint8 g, Uint8 b);

/*
 * Present the rendered frame (swap buffers)
 */
void renderer_present(renderer_t *rend);

/*
 * Update the window title (e.g., to show FPS)
 */
void renderer_set_title(renderer_t *rend, const char *title);

/*
 * Get the SDL_Renderer pointer for direct operations
 */
SDL_Renderer *renderer_get_sdl(renderer_t *rend);
