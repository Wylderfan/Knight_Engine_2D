/*
 * Knight Engine 2D - Main Entry Point
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>  /* STRESS_TEST - for rand() */
#include <string.h>
#include <math.h>

#include "core/config.h"
#include "graphics/camera.h"
#include "graphics/sprite.h"
#include "graphics/texture.h"
#include "input/input.h"
#include "input/input_config.h"
#include "util/debug.h"
#include "util/timer.h"

/*
 * Game state structure - holds all SDL resources and game data
 */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    texture_manager_t textures;
    input_state_t input;
    camera_t camera;
    sprite_t sprites[SPRITE_MAX_COUNT];
    int sprite_count;
    int player_index;  /* Index of player sprite in the array */
    SDL_Texture *background;
    bool running;
    /* Debug state */
    bool debug_enabled;
    Uint32 debug_last_output;  /* Last time debug info was printed */
    float debug_fps;           /* Current FPS for debug display */
    float debug_delta_time;    /* Current delta time for debug display */
    /* STRESS_TEST */
    bool stress_test_active;
    int stress_test_base_index;  /* First index of stress test sprites */
} game_state_t;

/* STRESS_TEST - Spawns/despawns test sprites for performance testing */
static void stress_test_toggle(game_state_t *game) {
    if (game->stress_test_active) {
        /* Despawn: destroy textures and reset count */
        for (int i = game->stress_test_base_index; i < game->sprite_count; i++) {
            if (game->sprites[i].texture) {
                SDL_DestroyTexture(game->sprites[i].texture);
            }
        }
        game->sprite_count = game->stress_test_base_index;
        game->stress_test_active = false;
        printf("[STRESS_TEST] Disabled - %d sprites now active\n", game->sprite_count);
    } else {
        /* Spawn stress test sprites */
        game->stress_test_base_index = game->sprite_count;
        int spawned = 0;
        for (int i = 0; i < STRESS_TEST_SPRITE_COUNT && game->sprite_count < SPRITE_MAX_COUNT; i++) {
            sprite_t *spr = &game->sprites[game->sprite_count++];
            spr->texture = texture_create_colored(game->renderer, 32, 32,
                (Uint8)(rand() % 256), (Uint8)(rand() % 256), (Uint8)(rand() % 256));
            /* Scatter across a larger world area */
            spr->x = (float)(rand() % (WINDOW_WIDTH * 2)) - WINDOW_WIDTH / 2;
            spr->y = (float)(rand() % (WINDOW_HEIGHT * 2)) - WINDOW_HEIGHT / 2;
            spr->vel_x = (float)(rand() % 100 - 50);  /* Random velocity */
            spr->vel_y = (float)(rand() % 100 - 50);
            spr->width = 32;
            spr->height = 32;
            spr->z_index = 30 + (rand() % 20);  /* z 30-49, below player */
            spr->angle = (double)(rand() % 360);
            spr->flip = SDL_FLIP_NONE;
            spr->show_debug_bounds = false;  /* Too cluttered with 100+ */
            spawned++;
        }
        game->stress_test_active = true;
        printf("[STRESS_TEST] Enabled - spawned %d sprites (%d total)\n",
               spawned, game->sprite_count);
    }
}

/*
 * Initialize SDL subsystems, create window and renderer
 *
 * The renderer is the core of SDL2's 2D rendering API.
 * It provides hardware-accelerated drawing operations and manages
 * textures on the GPU.
 */
static bool game_init(game_state_t *game) {
    /* Initialize SDL video subsystem */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    /* Initialize SDL_image for PNG loading (optional but nice to have) */
    int img_flags = IMG_INIT_PNG;
    if ((IMG_Init(img_flags) & img_flags) != img_flags) {
        fprintf(stderr, "SDL_image init warning: %s\n", IMG_GetError());
        /* Continue anyway - we can fall back to programmatic sprites */
    }

    /* Create the game window */
    game->window = SDL_CreateWindow(
        WINDOW_TITLE,                       /* title */
        SDL_WINDOWPOS_CENTERED,             /* x position */
        SDL_WINDOWPOS_CENTERED,             /* y position */
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN                    /* flags */
    );

    if (!game->window) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        return false;
    }

    /*
     * Create the renderer with hardware acceleration
     *
     * SDL_RENDERER_ACCELERATED: Use GPU for rendering
     * SDL_RENDERER_PRESENTVSYNC: Sync with monitor refresh rate
     */
    game->renderer = SDL_CreateRenderer(
        game->window,
        -1,  /* Use first available rendering driver */
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!game->renderer) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }

    /* Initialize texture manager */
    texture_manager_init(&game->textures, game->renderer);

    /* Initialize input system */
    input_init(&game->input);

    /* Initialize camera at origin */
    game->camera.x = 0.0f;
    game->camera.y = 0.0f;

    /* Initialize debug state */
    game->debug_enabled = false;
    game->debug_last_output = 0;
    game->debug_fps = 0.0f;
    game->debug_delta_time = 0.0f;

    /* STRESS_TEST - Initialize state */
    game->stress_test_active = false;
    game->stress_test_base_index = 0;

    /* Initialize sprite list */
    game->sprite_count = 0;

    /* Add player sprite (index 0) */
    sprite_t *player = &game->sprites[game->sprite_count++];
    game->player_index = 0;
    player->texture = texture_load(&game->textures, PLAYER_TEXTURE_PATH);
    if (!player->texture) {
        /* Fallback: create programmatic sprite */
        printf("Creating fallback player sprite\n");
        player->texture = texture_create_colored(game->renderer,
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
    player->z_index = 50;  /* Entity layer */
    player->angle = 0.0;
    player->flip = SDL_FLIP_NONE;
    player->show_debug_bounds = true;
    player->debug_r = 0;
    player->debug_g = 255;
    player->debug_b = 0;  /* Green border */

    /* Add test sprite (index 1) */
    sprite_t *test = &game->sprites[game->sprite_count++];
    test->texture = texture_create_colored(game->renderer,
        SPRITE_WIDTH, SPRITE_HEIGHT, 255, 100, 100);  /* Red-ish color */
    test->x = 100.0f;
    test->y = 100.0f;
    test->vel_x = 0.0f;
    test->vel_y = 0.0f;
    test->width = SPRITE_WIDTH;
    test->height = SPRITE_HEIGHT;
    test->z_index = 40;  /* Below player layer */
    test->angle = 45.0;  /* Rotated 45 degrees to demonstrate */
    test->flip = SDL_FLIP_NONE;
    test->show_debug_bounds = true;
    test->debug_r = 255;
    test->debug_g = 255;
    test->debug_b = 0;  /* Yellow border */

    /* Load background texture */
    game->background = texture_load(&game->textures, "assets/background.png");
    if (!game->background) {
        /* Fallback: create simple tiled background texture */
        printf("Creating fallback background\n");
        game->background = texture_create_colored(game->renderer,
            WINDOW_WIDTH, WINDOW_HEIGHT,
            COLOR_BG_R, COLOR_BG_G, COLOR_BG_B);
    }

    game->running = true;

    printf("Game initialized successfully\n");
    return true;
}

/*
 * Clean up all SDL resources
 * Always destroy in reverse order of creation
 */
static void game_cleanup(game_state_t *game) {
    /*
     * Clean up sprite textures not in manager (fallback/programmatic textures)
     * Textures loaded via texture_manager are cleaned up by texture_manager_cleanup
     */
    for (int i = 0; i < game->sprite_count; i++) {
        SDL_Texture *tex = game->sprites[i].texture;
        if (tex && i != game->player_index) {
            /* Non-player sprites use programmatic textures, destroy them */
            SDL_DestroyTexture(tex);
        } else if (tex && !texture_get(&game->textures, PLAYER_TEXTURE_PATH)) {
            /* Player fallback texture (not in manager) */
            SDL_DestroyTexture(tex);
        }
    }
    if (game->background &&
        !texture_get(&game->textures, "assets/background.png")) {
        SDL_DestroyTexture(game->background);
    }

    /* Clean up texture manager (handles all file-loaded textures) */
    texture_manager_cleanup(&game->textures);

    if (game->renderer) {
        SDL_DestroyRenderer(game->renderer);
    }
    if (game->window) {
        SDL_DestroyWindow(game->window);
    }

    IMG_Quit();
    SDL_Quit();

    printf("Game cleaned up\n");
}

/*
 * Process SDL events (window close, key presses, etc.)
 *
 * SDL uses an event queue - we poll events each frame and handle them.
 * This handles discrete events like key down/up, window close, etc.
 */
static void game_handle_events(game_state_t *game) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                /* Window close button clicked */
                game->running = false;
                break;

            case SDL_KEYDOWN:
                /* Handle single key press events */
                if (event.key.keysym.sym == KEY_QUIT ||
                    event.key.keysym.sym == KEY_QUIT_ALT) {
                    game->running = false;
                }
                break;
        }
    }
}

/*
 * Process continuous input (held keys)
 *
 * Uses the input system for clean key state checking.
 * input_key_down() checks if a key is currently held.
 */
static void game_process_input(game_state_t *game) {
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

/*
 * Update game logic
 *
 * delta_time is the time elapsed since last frame in seconds.
 * Multiplying velocity by delta_time ensures consistent movement
 * speed regardless of frame rate (velocity * time = distance).
 */
static void game_update(game_state_t *game, float delta_time) {
    sprite_t *player = &game->sprites[game->player_index];
    const input_state_t *input = &game->input;

    /* Toggle debug mode with P key */
    if (input_key_pressed(input, KEY_DEBUG_TOGGLE)) {
        game->debug_enabled = !game->debug_enabled;
        printf("[DEBUG] Debug mode %s\n", game->debug_enabled ? "ENABLED" : "DISABLED");
    }

    /* STRESS_TEST - Toggle with T key */
    if (input_key_pressed(input, KEY_STRESS_TEST)) {
        stress_test_toggle(game);
    }

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

    /* STRESS_TEST - Update stress test sprites */
    if (game->stress_test_active) {
        for (int i = game->stress_test_base_index; i < game->sprite_count; i++) {
            sprite_t *spr = &game->sprites[i];
            spr->x += spr->vel_x * delta_time;
            spr->y += spr->vel_y * delta_time;
            spr->angle += 90.0 * delta_time;  /* Rotate */
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

/*
 * Render the game
 *
 * Rendering in SDL2 follows this pattern:
 * 1. Clear the screen (set background color)
 * 2. Draw all game objects (textures, shapes, etc.)
 * 3. Present the frame (swap buffers to display)
 */
static void game_render(game_state_t *game) {
    /* Clear screen with background color */
    SDL_SetRenderDrawColor(game->renderer,
                           COLOR_BG_R, COLOR_BG_G, COLOR_BG_B, 255);
    SDL_RenderClear(game->renderer);

    /* Render background texture (covers full window) */
    if (game->background) {
        SDL_RenderCopy(game->renderer, game->background, NULL, NULL);
    }

    /* Render all sprites sorted by z_index (lower z renders first = behind) */
    for (int z = 0; z <= 100; z++) {
        for (int i = 0; i < game->sprite_count; i++) {
            sprite_t *spr = &game->sprites[i];
            if (spr->z_index == z) {
                if (spr->angle != 0.0 || spr->flip != SDL_FLIP_NONE) {
                    sprite_render_ex(game->renderer, spr, &game->camera, NULL,
                                     spr->angle, NULL, spr->flip,
                                     255, 255, 255);
                } else {
                    sprite_render(game->renderer, spr, &game->camera, NULL);
                }

                /* Draw debug bounds when debug mode is enabled */
                if (game->debug_enabled && spr->show_debug_bounds) {
                    debug_draw_rect_rotated(game->renderer, &game->camera,
                                            spr->x, spr->y, spr->width, spr->height,
                                            spr->angle,
                                            spr->debug_r, spr->debug_g, spr->debug_b, 255);
                }
            }
        }
    }

    /* Present the frame - this displays everything we've drawn */
    SDL_RenderPresent(game->renderer);
}

/*
 * Main entry point
 *
 * The game loop is the heart of any game:
 * 1. Handle events (user input, window events)
 * 2. Update game state (physics, logic)
 * 3. Render (draw everything)
 * 4. Control frame timing
 */
int main(int argc, char *argv[]) {
    (void)argc;  /* Unused */
    (void)argv;  /* Unused */

    game_state_t game = {0};

    if (!game_init(&game)) {
        game_cleanup(&game);
        return 1;
    }

    printf("Controls: Arrow keys or WASD to move, P=debug, T=stress test, ESC to quit\n");

    Uint32 last_time = SDL_GetTicks();

    /* FPS counter variables */
    Uint32 fps_last_time = SDL_GetTicks();
    int frame_count = 0;
    float current_fps = 0.0f;

    /*
     * Fixed timestep accumulator pattern
     *
     * Accumulates frame time and runs physics updates at a fixed rate.
     * This ensures consistent physics behavior regardless of frame rate.
     * Benefits:
     * - Deterministic physics (same results every time)
     * - Stable collision detection
     * - No physics glitches on slow frames
     */
    float accumulator = 0.0f;

    /* Main game loop */
    while (game.running) {
        /* Calculate delta time (time since last frame in seconds) */
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        /* Cap delta time to prevent huge jumps (e.g., after breakpoints) */
        if (delta_time > MAX_DELTA_TIME) {
            delta_time = MAX_DELTA_TIME;
        }

        /* FPS calculation and display */
        frame_count++;
#if FPS_DISPLAY_ENABLED || FPS_DEBUG_LOG
        if (current_time - fps_last_time >= FPS_UPDATE_INTERVAL) {
            current_fps = frame_count * 1000.0f / (current_time - fps_last_time);
            frame_count = 0;
            fps_last_time = current_time;

#if FPS_DISPLAY_ENABLED
            /* Display FPS in window title */
            char title_buffer[128];
            snprintf(title_buffer, sizeof(title_buffer), "%s - %.1f FPS",
                     WINDOW_TITLE, current_fps);
            SDL_SetWindowTitle(game.window, title_buffer);
#endif

#if FPS_DEBUG_LOG
            /* Log actual vs target FPS to console */
            float fps_diff = current_fps - TARGET_FPS;
            printf("[FPS] Actual: %.1f | Target: %d | Diff: %+.1f\n",
                   current_fps, TARGET_FPS, fps_diff);
#endif
        }
#endif

        /* Store debug values */
        game.debug_fps = current_fps;
        game.debug_delta_time = delta_time;

        /* Debug output (when enabled) */
        if (game.debug_enabled &&
            current_time - game.debug_last_output >= DEBUG_OUTPUT_INTERVAL) {
            game.debug_last_output = current_time;
            printf("[DEBUG] FPS: %.1f | Delta: %.4fs (%.2fms) | "
                   "Sprites: %d | Player: (%.1f, %.1f) | Camera: (%.1f, %.1f)\n",
                   game.debug_fps,
                   game.debug_delta_time,
                   game.debug_delta_time * 1000.0f,
                   game.sprite_count,
                   game.sprites[game.player_index].x,
                   game.sprites[game.player_index].y,
                   game.camera.x,
                   game.camera.y);
        }

        /* Update input state (must be called before processing input) */
        input_update(&game.input);

        /* Handle events and input (once per frame) */
        game_handle_events(&game);
        game_process_input(&game);

        /*
         * Fixed timestep update loop
         * Accumulate time and run updates at fixed intervals.
         * This decouples physics from rendering frame rate.
         */
        accumulator += delta_time;
        if (accumulator > MAX_ACCUMULATOR) {
            accumulator = MAX_ACCUMULATOR;  /* Prevent spiral of death */
        }
        while (accumulator >= FIXED_TIMESTEP) {
            game_update(&game, FIXED_TIMESTEP);
            accumulator -= FIXED_TIMESTEP;
        }

        /* Render (once per frame, after all updates) */
        game_render(&game);

        /*
         * Frame rate is controlled by VSYNC (SDL_RENDERER_PRESENTVSYNC).
         * SDL_RenderPresent blocks until the next monitor refresh.
         */
    }

    (void)current_fps; /* Suppress unused warning when FPS disabled */

    game_cleanup(&game);
    return 0;
}
