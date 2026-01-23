/*
 * Knight Engine 2D - Renderer System Implementation
 */

#include "graphics/renderer.h"
#include <SDL2/SDL_image.h>
#include <stdio.h>

bool renderer_init(renderer_t *rend, const char *title, int width, int height) {
    rend->width = width;
    rend->height = height;
    rend->window = NULL;
    rend->renderer = NULL;

    /* Initialize SDL video subsystem */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    /* Initialize SDL_image for PNG loading */
    int img_flags = IMG_INIT_PNG;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        fprintf(stderr, "SDL_image init warning: %s\n", IMG_GetError());
        /* Continue anyway - can fall back to programmatic sprites */
    }

    /* Create the game window */
    rend->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );

    if (!rend->window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        return false;
    }

    /* Create the renderer with hardware acceleration and VSYNC */
    rend->renderer = SDL_CreateRenderer(
        rend->window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!rend->renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void renderer_cleanup(renderer_t *rend) {
    if (rend->renderer) {
        SDL_DestroyRenderer(rend->renderer);
        rend->renderer = NULL;
    }
    if (rend->window) {
        SDL_DestroyWindow(rend->window);
        rend->window = NULL;
    }

    IMG_Quit();
    SDL_Quit();
}

void renderer_clear(renderer_t *rend, Uint8 r, Uint8 g, Uint8 b) {
    SDL_SetRenderDrawColor(rend->renderer, r, g, b, 255);
    SDL_RenderClear(rend->renderer);
}

void renderer_present(renderer_t *rend) {
    SDL_RenderPresent(rend->renderer);
}

void renderer_set_title(renderer_t *rend, const char *title) {
    SDL_SetWindowTitle(rend->window, title);
}

SDL_Renderer *renderer_get_sdl(renderer_t *rend) {
    return rend->renderer;
}
