/*
 * Knight Engine 2D - Texture Management
 *
 * Handles texture loading, caching, and cleanup.
 * Prevents loading the same texture multiple times.
 */

#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "core/config.h"

/*
 * Texture entry - stores a loaded texture with its path identifier
 */
typedef struct {
    char path[TEXTURE_PATH_MAX_LEN];
    SDL_Texture *texture;
    int width;
    int height;
} texture_entry_t;

/*
 * Texture manager - simple storage for loaded textures
 */
typedef struct {
    texture_entry_t entries[TEXTURE_MAX_ENTRIES];
    int count;
    SDL_Renderer *renderer;
} texture_manager_t;

/*
 * Initialize the texture manager
 */
void texture_manager_init(texture_manager_t *tm, SDL_Renderer *renderer);

/*
 * Load a texture from file path
 * Returns the loaded texture, or NULL on failure.
 * Caches textures - subsequent loads of the same path return cached version.
 */
SDL_Texture *texture_load(texture_manager_t *tm, const char *path);

/*
 * Get a previously loaded texture by path
 * Returns NULL if not found (use texture_load to load first)
 */
SDL_Texture *texture_get(texture_manager_t *tm, const char *path);

/*
 * Get texture dimensions by path
 * Returns true if found, false otherwise
 */
bool texture_get_size(texture_manager_t *tm, const char *path,
                      int *width, int *height);

/*
 * Clean up all loaded textures
 */
void texture_manager_cleanup(texture_manager_t *tm);

/*
 * Create a colored rectangle texture programmatically
 *
 * SDL surfaces are CPU-side image data that can be manipulated directly.
 * SDL textures are GPU-side and optimized for rendering.
 */
SDL_Texture *texture_create_colored(SDL_Renderer *renderer,
                                    int width, int height,
                                    Uint8 r, Uint8 g, Uint8 b);
