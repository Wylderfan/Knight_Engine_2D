/*
 * Knight Engine 2D - Sprite System Implementation
 */

#include "graphics/sprite.h"
#include "graphics/camera.h"

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
