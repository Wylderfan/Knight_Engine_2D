/*
 * Knight Engine 2D - Engine Implementation
 */

#include "core/engine.h"
#include "core/config.h"
#include "core/game_logic.h"
#include "core/game_state.h"
#include "graphics/renderer.h"
#include "graphics/sprite.h"
#include "graphics/texture.h"
#include "input/input.h"
#include "input/input_config.h"
#include "util/debug.h"
#include "util/timer.h"
#include <SDL2/SDL.h>
#include <stdio.h>

/*
 * Process SDL events (window close, key presses, etc.)
 */
static void engine_handle_events(game_state_t *game) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                game->running = false;
                break;

            case SDL_KEYDOWN:
                if (event.key.keysym.sym == KEY_QUIT ||
                    event.key.keysym.sym == KEY_QUIT_ALT) {
                    game->running = false;
                }
                break;
        }
    }
}

/*
 * Render the game
 */
static void engine_render(game_state_t *game) {
    SDL_Renderer *sdl_renderer = renderer_get_sdl(&game->renderer);

    renderer_clear(&game->renderer, COLOR_BG_R, COLOR_BG_G, COLOR_BG_B);

    if (game->background) {
        SDL_RenderCopy(sdl_renderer, game->background, NULL, NULL);
    }

    /* Render all sprites sorted by z_index */
    for (int z = 0; z <= 100; z++) {
        for (int i = 0; i < game->sprite_count; i++) {
            sprite_t *spr = &game->sprites[i];
            if (spr->z_index == z) {
                if (spr->angle != 0.0 || spr->flip != SDL_FLIP_NONE) {
                    sprite_render_ex(sdl_renderer, spr, &game->camera, NULL,
                                     spr->angle, NULL, spr->flip,
                                     255, 255, 255);
                } else {
                    sprite_render(sdl_renderer, spr, &game->camera, NULL);
                }

                if (game->debug_enabled && spr->show_debug_bounds) {
                    debug_draw_rect_rotated(sdl_renderer, &game->camera,
                                            spr->x, spr->y, spr->width, spr->height,
                                            spr->angle,
                                            spr->debug_r, spr->debug_g, spr->debug_b, 255);
                }
            }
        }
    }

    renderer_present(&game->renderer);
}

bool engine_init(game_state_t *game) {
    /* Initialize rendering system */
    if (!renderer_init(&game->renderer, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT)) {
        return false;
    }

    /* Initialize texture manager */
    texture_manager_init(&game->textures, renderer_get_sdl(&game->renderer));

    /* Initialize input system */
    input_init(&game->input);

    /* Initialize FPS counter */
    fps_counter_init(&game->fps);

    /* Initialize camera at origin */
    game->camera.x = 0.0f;
    game->camera.y = 0.0f;

    /* Initialize debug state */
    game->debug_enabled = false;
    game->debug_last_output = 0;
    game->debug_fps = 0.0f;
    game->debug_delta_time = 0.0f;

    /* Initialize stress test state */
    game->stress_test_active = false;
    game->stress_test_base_index = 0;

    /* Initialize sprite list */
    game->sprite_count = 0;

    /* Add player sprite (index 0) */
    sprite_t *player = &game->sprites[game->sprite_count++];
    game->player_index = 0;
    player->texture = texture_load(&game->textures, PLAYER_TEXTURE_PATH);
    if (!player->texture) {
        printf("Creating fallback player sprite\n");
        player->texture = texture_create_colored(renderer_get_sdl(&game->renderer),
            SPRITE_WIDTH, SPRITE_HEIGHT,
            COLOR_PLAYER_R, COLOR_PLAYER_G, COLOR_PLAYER_B);
        if (!player->texture) {
            fprintf(stderr, "Failed to create player texture\n");
            return false;
        }
    }
    player->x = PLAYER_START_X;
    player->y = PLAYER_START_Y;
    player->vel_x = 0.0f;
    player->vel_y = 0.0f;
    player->width = SPRITE_WIDTH;
    player->height = SPRITE_HEIGHT;
    player->z_index = 50;
    player->angle = 0.0;
    player->flip = SDL_FLIP_NONE;
    player->show_debug_bounds = true;
    player->debug_r = 0;
    player->debug_g = 255;
    player->debug_b = 0;

    /* Add test sprite (index 1) */
    sprite_t *test = &game->sprites[game->sprite_count++];
    test->texture = texture_create_colored(renderer_get_sdl(&game->renderer),
        SPRITE_WIDTH, SPRITE_HEIGHT, 255, 100, 100);
    test->x = 100.0f;
    test->y = 100.0f;
    test->vel_x = 0.0f;
    test->vel_y = 0.0f;
    test->width = SPRITE_WIDTH;
    test->height = SPRITE_HEIGHT;
    test->z_index = 40;
    test->angle = 45.0;
    test->flip = SDL_FLIP_NONE;
    test->show_debug_bounds = true;
    test->debug_r = 255;
    test->debug_g = 255;
    test->debug_b = 0;

    /* Load background texture */
    game->background = texture_load(&game->textures, "assets/background.png");
    if (!game->background) {
        printf("Creating fallback background\n");
        game->background = texture_create_colored(renderer_get_sdl(&game->renderer),
            WINDOW_WIDTH, WINDOW_HEIGHT,
            COLOR_BG_R, COLOR_BG_G, COLOR_BG_B);
    }

    game->running = true;

    printf("Game initialized successfully\n");
    return true;
}

void engine_cleanup(game_state_t *game) {
    /* Clean up sprite textures not in manager */
    for (int i = 0; i < game->sprite_count; i++) {
        SDL_Texture *tex = game->sprites[i].texture;
        if (tex && i != game->player_index) {
            SDL_DestroyTexture(tex);
        } else if (tex && !texture_get(&game->textures, PLAYER_TEXTURE_PATH)) {
            SDL_DestroyTexture(tex);
        }
    }
    if (game->background &&
        !texture_get(&game->textures, BACKGROUND_TEXTURE_PATH)) {
        SDL_DestroyTexture(game->background);
    }

    texture_manager_cleanup(&game->textures);
    renderer_cleanup(&game->renderer);

    printf("Game cleaned up\n");
}

void engine_run(game_state_t *game) {
    printf("Controls: Arrow keys or WASD to move, P=debug, T=stress test, ESC to quit\n");

    /* Re-initialize FPS counter at loop start for accurate timing */
    fps_counter_init(&game->fps);

    Uint32 last_time = SDL_GetTicks();
    float accumulator = 0.0f;

    while (game->running) {
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        if (delta_time > MAX_DELTA_TIME) {
            delta_time = MAX_DELTA_TIME;
        }

        /* Update FPS counter (debug feature) */
        if (fps_counter_update(&game->fps, current_time)) {
            float current_fps = fps_counter_get(&game->fps);

#if FPS_DISPLAY_ENABLED
            char title_buffer[128];
            snprintf(title_buffer, sizeof(title_buffer), "%s - %.1f FPS",
                     WINDOW_TITLE, current_fps);
            renderer_set_title(&game->renderer, title_buffer);
#endif

#if FPS_DEBUG_LOG
            float fps_diff = current_fps - TARGET_FPS;
            printf("[FPS] Actual: %.1f | Target: %d | Diff: %+.1f\n",
                   current_fps, TARGET_FPS, fps_diff);
#endif
        }

        /* Store debug values */
        game->debug_fps = fps_counter_get(&game->fps);
        game->debug_delta_time = delta_time;

        /* Debug output (when enabled) */
        if (game->debug_enabled &&
            current_time - game->debug_last_output >= DEBUG_OUTPUT_INTERVAL) {
            game->debug_last_output = current_time;
            printf("[DEBUG] FPS: %.1f | Delta: %.4fs (%.2fms) | "
                   "Sprites: %d | Player: (%.1f, %.1f) | Camera: (%.1f, %.1f)\n",
                   game->debug_fps,
                   game->debug_delta_time,
                   game->debug_delta_time * 1000.0f,
                   game->sprite_count,
                   game->sprites[game->player_index].x,
                   game->sprites[game->player_index].y,
                   game->camera.x,
                   game->camera.y);
        }

        input_update(&game->input);
        engine_handle_events(game);
        game_process_input(game);

        /* Handle discrete input (toggles) - must be per-frame, not fixed timestep */
        if (input_key_pressed(&game->input, KEY_DEBUG_TOGGLE)) {
            game->debug_enabled = !game->debug_enabled;
            printf("[DEBUG] Debug mode %s\n", game->debug_enabled ? "ENABLED" : "DISABLED");
        }
        if (input_key_pressed(&game->input, KEY_STRESS_TEST)) {
            debug_stress_test_toggle(game);
        }

        /* Fixed timestep update loop */
        accumulator += delta_time;
        if (accumulator > MAX_ACCUMULATOR) {
            accumulator = MAX_ACCUMULATOR;
        }
        while (accumulator >= FIXED_TIMESTEP) {
            game_update(game, FIXED_TIMESTEP);
            accumulator -= FIXED_TIMESTEP;
        }

        engine_render(game);
    }
}
