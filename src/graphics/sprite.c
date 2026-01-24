/*
 * Knight Engine 2D - Sprite System Implementation
 */

#include "graphics/sprite.h"
#include "graphics/camera.h"

/* Quicksort partition for sprite indices by z_index */
static int partition(const sprite_t *sprites, int *indices, int low, int high) {
    int pivot_z = sprites[indices[high]].z_index;
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (sprites[indices[j]].z_index <= pivot_z) {
            i++;
            int temp = indices[i];
            indices[i] = indices[j];
            indices[j] = temp;
        }
    }

    int temp = indices[i + 1];
    indices[i + 1] = indices[high];
    indices[high] = temp;

    return i + 1;
}

/* Recursive quicksort for sprite indices */
static void quicksort(const sprite_t *sprites, int *indices, int low, int high) {
    if (low < high) {
        int pivot = partition(sprites, indices, low, high);
        quicksort(sprites, indices, low, pivot - 1);
        quicksort(sprites, indices, pivot + 1, high);
    }
}

void sprite_render(SDL_Renderer *renderer, const sprite_t *sprite,
                   const camera_t *camera, const SDL_Rect *src_rect) {
    if (!sprite->texture) {
        return;
    }

    int screen_x, screen_y;
    world_to_screen(camera, sprite->x, sprite->y, &screen_x, &screen_y);

    SDL_Rect dest_rect = {
        screen_x,
        screen_y,
        sprite->width,
        sprite->height
    };

    SDL_RenderCopy(renderer, sprite->texture, src_rect, &dest_rect);
}

void sprite_render_ex(SDL_Renderer *renderer, const sprite_t *sprite,
                      const camera_t *camera, const SDL_Rect *src_rect,
                      double angle, const SDL_Point *center,
                      SDL_RendererFlip flip, Uint8 r, Uint8 g, Uint8 b) {
    if (!sprite->texture) {
        return;
    }

    int screen_x, screen_y;
    world_to_screen(camera, sprite->x, sprite->y, &screen_x, &screen_y);

    SDL_Rect dest_rect = {
        screen_x,
        screen_y,
        sprite->width,
        sprite->height
    };

    /* Apply color modulation */
    SDL_SetTextureColorMod(sprite->texture, r, g, b);

    SDL_RenderCopyEx(renderer, sprite->texture, src_rect, &dest_rect,
                     angle, center, flip);

    /* Reset color modulation to default */
    SDL_SetTextureColorMod(sprite->texture, 255, 255, 255);
}

void sprite_sort_by_z(const sprite_t *sprites, int *render_order, int count) {
    if (count <= 0) {
        return;
    }

    /* Initialize indices */
    for (int i = 0; i < count; i++) {
        render_order[i] = i;
    }

    /* Sort indices by z_index */
    quicksort(sprites, render_order, 0, count - 1);
}
