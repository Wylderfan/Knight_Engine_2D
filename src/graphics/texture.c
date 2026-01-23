/*
 * Knight Engine 2D - Texture Management Implementation
 */

#include "graphics/texture.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>

void texture_manager_init(texture_manager_t *tm, SDL_Renderer *renderer) {
    tm->count = 0;
    tm->renderer = renderer;
    for (int i = 0; i < TEXTURE_MAX_ENTRIES; i++) {
        tm->entries[i].path[0] = '\0';
        tm->entries[i].texture = NULL;
        tm->entries[i].width = 0;
        tm->entries[i].height = 0;
    }
}

SDL_Texture *texture_load(texture_manager_t *tm, const char *path) {
    /* Check if already loaded */
    for (int i = 0; i < tm->count; i++) {
        if (strcmp(tm->entries[i].path, path) == 0) {
            return tm->entries[i].texture;
        }
    }

    /* Check capacity */
    if (tm->count >= TEXTURE_MAX_ENTRIES) {
        fprintf(stderr, "Texture manager full, cannot load: %s\n", path);
        return NULL;
    }

    /* Load the texture */
    SDL_Texture *texture = IMG_LoadTexture(tm->renderer, path);
    if (!texture) {
        fprintf(stderr, "Failed to load texture '%s': %s\n", path, IMG_GetError());
        return NULL;
    }

    /* Enable alpha blending */
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    /* Query texture dimensions */
    int width, height;
    SDL_QueryTexture(texture, NULL, NULL, &width, &height);

    /* Store in cache */
    texture_entry_t *entry = &tm->entries[tm->count];
    strncpy(entry->path, path, TEXTURE_PATH_MAX_LEN - 1);
    entry->path[TEXTURE_PATH_MAX_LEN - 1] = '\0';
    entry->texture = texture;
    entry->width = width;
    entry->height = height;
    tm->count++;

    printf("Loaded texture: %s (%dx%d)\n", path, width, height);
    return texture;
}

SDL_Texture *texture_get(texture_manager_t *tm, const char *path) {
    for (int i = 0; i < tm->count; i++) {
        if (strcmp(tm->entries[i].path, path) == 0) {
            return tm->entries[i].texture;
        }
    }
    return NULL;
}

bool texture_get_size(texture_manager_t *tm, const char *path,
                      int *width, int *height) {
    for (int i = 0; i < tm->count; i++) {
        if (strcmp(tm->entries[i].path, path) == 0) {
            *width = tm->entries[i].width;
            *height = tm->entries[i].height;
            return true;
        }
    }
    return false;
}

void texture_manager_cleanup(texture_manager_t *tm) {
    for (int i = 0; i < tm->count; i++) {
        if (tm->entries[i].texture) {
            SDL_DestroyTexture(tm->entries[i].texture);
            tm->entries[i].texture = NULL;
        }
        tm->entries[i].path[0] = '\0';
    }
    tm->count = 0;
    printf("Texture manager cleaned up\n");
}

SDL_Texture *texture_create_colored(SDL_Renderer *renderer,
                                    int width, int height,
                                    Uint8 r, Uint8 g, Uint8 b) {
    /* Create a 32-bit RGBA surface */
    SDL_Surface *surface = SDL_CreateRGBSurface(
        0,              /* flags (unused) */
        width, height,  /* dimensions */
        32,             /* bits per pixel */
        0x00FF0000,     /* red mask */
        0x0000FF00,     /* green mask */
        0x000000FF,     /* blue mask */
        0xFF000000      /* alpha mask */
    );

    if (!surface) {
        fprintf(stderr, "Failed to create surface: %s\n", SDL_GetError());
        return NULL;
    }

    /* Fill the surface with the specified color */
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, r, g, b));

    /* Convert surface to texture for GPU-accelerated rendering */
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    /* Free the surface - we only need the texture now */
    SDL_FreeSurface(surface);

    if (!texture) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
    }

    return texture;
}
