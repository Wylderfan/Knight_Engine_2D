/*
 * Knight Engine 2D - Simple SDL2 Sprite Demo
 *
 * This demonstrates basic SDL2 concepts:
 * - Window and renderer creation
 * - Texture loading and rendering
 * - Game loop structure (events -> update -> render)
 * - Keyboard input handling
 * - Frame rate control
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>

/* ============================================================================
 * CONFIGURATION - Adjust these values to customize game behavior
 * ============================================================================ */

/* Window settings */
#define WINDOW_TITLE  "Knight Engine 2D - Sprite Demo"
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

/* Sprite properties */
#define SPRITE_WIDTH  64
#define SPRITE_HEIGHT 64
#define SPRITE_SPEED  300.0f  /* Pixels per second */

/* Player starting position (center of screen if not specified) */
#define PLAYER_START_X ((WINDOW_WIDTH - SPRITE_WIDTH) / 2.0f)
#define PLAYER_START_Y ((WINDOW_HEIGHT - SPRITE_HEIGHT) / 2.0f)

/* Frame rate control */
#define TARGET_FPS        60
#define FIXED_TIMESTEP    (1.0f / TARGET_FPS)  /* Fixed update rate for physics */
#define MAX_DELTA_TIME    0.1f   /* Cap delta time to prevent large jumps */
#define MAX_ACCUMULATOR   0.25f  /* Prevent spiral of death on slow frames */

/* FPS counter settings */
#define FPS_UPDATE_INTERVAL 500  /* Update FPS display every N milliseconds */
#define FPS_DISPLAY_ENABLED 1    /* Set to 0 to disable FPS in window title */
#define FPS_DEBUG_LOG       0    /* Set to 1 to log FPS vs target to console */

/* Asset paths */
#define PLAYER_TEXTURE_PATH "assets/player.png"

/* Background color (RGB) - grass green */
#define COLOR_BG_R 34
#define COLOR_BG_G 139
#define COLOR_BG_B 34

/* Player sprite fallback color (RGB) - royal blue */
#define COLOR_PLAYER_R 65
#define COLOR_PLAYER_G 105
#define COLOR_PLAYER_B 225

/* ============================================================================
 * KEY BINDINGS - Customize controls here
 * Uses SDL_SCANCODE_* values (physical key positions)
 * ============================================================================ */

/* Movement keys (primary) */
#define KEY_MOVE_UP     SDL_SCANCODE_UP
#define KEY_MOVE_DOWN   SDL_SCANCODE_DOWN
#define KEY_MOVE_LEFT   SDL_SCANCODE_LEFT
#define KEY_MOVE_RIGHT  SDL_SCANCODE_RIGHT

/* Movement keys (alternate - WASD) */
#define KEY_MOVE_UP_ALT     SDL_SCANCODE_W
#define KEY_MOVE_DOWN_ALT   SDL_SCANCODE_S
#define KEY_MOVE_LEFT_ALT   SDL_SCANCODE_A
#define KEY_MOVE_RIGHT_ALT  SDL_SCANCODE_D

/* Menu/system keys (uses SDLK_* for key symbols) */
#define KEY_QUIT SDLK_ESCAPE

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
    SDL_Texture *texture;
} sprite_t;

/*
 * Game state structure - holds all SDL resources and game data
 */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    sprite_t player;
    bool running;
} game_state_t;

/*
 * Render a sprite to the screen
 * Call this for each sprite during the render phase.
 */
static void sprite_render(SDL_Renderer *renderer, const sprite_t *sprite) {
    if (!sprite->texture) {
        return;
    }

    SDL_Rect dest_rect = {
        (int)sprite->x,
        (int)sprite->y,
        sprite->width,
        sprite->height
    };

    SDL_RenderCopy(renderer, sprite->texture, NULL, &dest_rect);
}

/*
 * Create a colored rectangle texture programmatically
 *
 * SDL surfaces are CPU-side image data that can be manipulated directly.
 * SDL textures are GPU-side and optimized for rendering.
 * We create a surface, fill it with color, then convert to texture.
 */
static SDL_Texture *create_colored_texture(SDL_Renderer *renderer,
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

/*
 * Load player texture - tries to load from file first, falls back to
 * programmatically generated colored rectangle
 */
static SDL_Texture *load_player_texture(SDL_Renderer *renderer) {
    SDL_Texture *texture = NULL;

    /* Try to load texture from configured path */
    texture = IMG_LoadTexture(renderer, PLAYER_TEXTURE_PATH);

    if (texture) {
        /* Enable alpha blending for transparency */
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        printf("Loaded player texture from %s\n", PLAYER_TEXTURE_PATH);
        return texture;
    }

    /* Fallback: create a colored rectangle */
    printf("%s not found, creating programmatic sprite\n", PLAYER_TEXTURE_PATH);
    texture = create_colored_texture(renderer, SPRITE_WIDTH, SPRITE_HEIGHT,
                                     COLOR_PLAYER_R, COLOR_PLAYER_G, COLOR_PLAYER_B);

    return texture;
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

    /* Initialize player sprite */
    game->player.texture = load_player_texture(game->renderer);
    if (!game->player.texture) {
        fprintf(stderr, "Failed to load player texture\n");
        return false;
    }
    game->player.x = PLAYER_START_X;
    game->player.y = PLAYER_START_Y;
    game->player.vel_x = 0.0f;
    game->player.vel_y = 0.0f;
    game->player.width = SPRITE_WIDTH;
    game->player.height = SPRITE_HEIGHT;

    game->running = true;

    printf("Game initialized successfully\n");
    return true;
}

/*
 * Clean up all SDL resources
 * Always destroy in reverse order of creation
 */
static void game_cleanup(game_state_t *game) {
    if (game->player.texture) {
        SDL_DestroyTexture(game->player.texture);
    }
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
                if (event.key.keysym.sym == KEY_QUIT) {
                    game->running = false;
                }
                break;
        }
    }
}

/*
 * Process continuous input (held keys)
 *
 * SDL_GetKeyboardState returns a snapshot of all keyboard keys.
 * This is better for movement than events because it handles
 * held keys smoothly without relying on key repeat.
 */
static void game_process_input(game_state_t *game) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    /* Reset velocity each frame */
    game->player.vel_x = 0.0f;
    game->player.vel_y = 0.0f;

    /* Check movement keys and set velocity */
    if (keys[KEY_MOVE_UP] || keys[KEY_MOVE_UP_ALT]) {
        game->player.vel_y = -SPRITE_SPEED;
    }
    if (keys[KEY_MOVE_DOWN] || keys[KEY_MOVE_DOWN_ALT]) {
        game->player.vel_y = SPRITE_SPEED;
    }
    if (keys[KEY_MOVE_LEFT] || keys[KEY_MOVE_LEFT_ALT]) {
        game->player.vel_x = -SPRITE_SPEED;
    }
    if (keys[KEY_MOVE_RIGHT] || keys[KEY_MOVE_RIGHT_ALT]) {
        game->player.vel_x = SPRITE_SPEED;
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
    /* Update player position based on velocity and delta time */
    game->player.x += game->player.vel_x * delta_time;
    game->player.y += game->player.vel_y * delta_time;

    /* Clamp player position to screen boundaries */
    if (game->player.x < 0) {
        game->player.x = 0;
    }
    if (game->player.x > WINDOW_WIDTH - game->player.width) {
        game->player.x = WINDOW_WIDTH - game->player.width;
    }
    if (game->player.y < 0) {
        game->player.y = 0;
    }
    if (game->player.y > WINDOW_HEIGHT - game->player.height) {
        game->player.y = WINDOW_HEIGHT - game->player.height;
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
    /* Set background color and clear screen */
    SDL_SetRenderDrawColor(game->renderer,
                           COLOR_BG_R, COLOR_BG_G, COLOR_BG_B, 255);
    SDL_RenderClear(game->renderer);

    /* Render all sprites */
    sprite_render(game->renderer, &game->player);

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

    printf("Controls: Arrow keys or WASD to move, ESC to quit\n");

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
