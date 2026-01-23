/*
 * Knight Engine 2D - Game Logic Implementation
 */

#include "core/game_logic.h"
#include "core/config.h"
#include "core/game_state.h"
#include "graphics/sprite.h"
#include "input/input.h"
#include "input/input_config.h"

void game_process_input(game_state_t *game) {
    const input_state_t *input = &game->input;
    sprite_t *player = &game->sprites[game->player_index];

    /* Reset velocity each frame */
    player->vel_x = 0.0f;
    player->vel_y = 0.0f;

    /* Check movement keys and set velocity */
    if (input_key_down(input, KEY_MOVE_UP) ||
        input_key_down(input, KEY_MOVE_UP_ALT)) {
        player->vel_y = -SPRITE_SPEED;
    }
    if (input_key_down(input, KEY_MOVE_DOWN) ||
        input_key_down(input, KEY_MOVE_DOWN_ALT)) {
        player->vel_y = SPRITE_SPEED;
    }
    if (input_key_down(input, KEY_MOVE_LEFT) ||
        input_key_down(input, KEY_MOVE_LEFT_ALT)) {
        player->vel_x = -SPRITE_SPEED;
    }
    if (input_key_down(input, KEY_MOVE_RIGHT) ||
        input_key_down(input, KEY_MOVE_RIGHT_ALT)) {
        player->vel_x = SPRITE_SPEED;
    }
}

void game_update(game_state_t *game, float delta_time) {
    sprite_t *player = &game->sprites[game->player_index];
    const input_state_t *input = &game->input;

    /* Update camera position (IJKL keys) */
    if (input_key_down(input, KEY_CAM_UP)) {
        game->camera.y -= CAMERA_SPEED * delta_time;
    }
    if (input_key_down(input, KEY_CAM_DOWN)) {
        game->camera.y += CAMERA_SPEED * delta_time;
    }
    if (input_key_down(input, KEY_CAM_LEFT)) {
        game->camera.x -= CAMERA_SPEED * delta_time;
    }
    if (input_key_down(input, KEY_CAM_RIGHT)) {
        game->camera.x += CAMERA_SPEED * delta_time;
    }

    /* Update player position based on velocity and delta time */
    player->x += player->vel_x * delta_time;
    player->y += player->vel_y * delta_time;

    /* Update stress test sprites */
    if (game->stress_test_active) {
        for (int i = game->stress_test_base_index; i < game->sprite_count; i++) {
            sprite_t *spr = &game->sprites[i];
            spr->x += spr->vel_x * delta_time;
            spr->y += spr->vel_y * delta_time;
            spr->angle += 90.0 * delta_time;
            /* Bounce off world bounds */
            if (spr->x < -WINDOW_WIDTH || spr->x > WINDOW_WIDTH * 2) spr->vel_x = -spr->vel_x;
            if (spr->y < -WINDOW_HEIGHT || spr->y > WINDOW_HEIGHT * 2) spr->vel_y = -spr->vel_y;
        }
    }

    /* Clamp player position to camera's visible area (world coordinates) */
    float cam_left = game->camera.x;
    float cam_right = game->camera.x + WINDOW_WIDTH - player->width;
    float cam_top = game->camera.y;
    float cam_bottom = game->camera.y + WINDOW_HEIGHT - player->height;

    if (player->x < cam_left) {
        player->x = cam_left;
    }
    if (player->x > cam_right) {
        player->x = cam_right;
    }
    if (player->y < cam_top) {
        player->y = cam_top;
    }
    if (player->y > cam_bottom) {
        player->y = cam_bottom;
    }
}
