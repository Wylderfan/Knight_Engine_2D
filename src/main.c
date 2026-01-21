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

/* Window dimensions */
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

/* Sprite properties */
#define SPRITE_WIDTH  64
#define SPRITE_HEIGHT 64
#define SPRITE_SPEED  300.0f  /* Pixels per second */

/* Frame rate control */
#define TARGET_FPS    60
#define FRAME_DELAY   (1000 / TARGET_FPS)

/* Colors (RGBA format) */
#define COLOR_GRASS_R 34
#define COLOR_GRASS_G 139
#define COLOR_GRASS_B 34

#define COLOR_PLAYER_R 65
#define COLOR_PLAYER_G 105
#define COLOR_PLAYER_B 225

/*
 * Player structure - holds position and velocity
 * Using floats for smooth sub-pixel movement
 */
typedef struct {
    float x;
    float y;
    float vel_x;
    float vel_y;
    int width;
    int height;
} player_t;

/*
 * Game state structure - holds all SDL resources and game data
 */
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *player_texture;
    player_t player;
    bool running;
} game_state_t;

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

    /* Try to load player.png from assets folder */
    texture = IMG_LoadTexture(renderer, "assets/player.png");

    if (texture) {
        printf("Loaded player texture from assets/player.png\n");
        return texture;
    }

    /* Fallback: create a colored rectangle */
    printf("player.png not found, creating programmatic sprite\n");
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
        "Knight Engine 2D - Sprite Demo",   /* title */
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

    /* Load the player texture */
    game->player_texture = load_player_texture(game->renderer);
    if (!game->player_texture) {
        fprintf(stderr, "Failed to load player texture\n");
        return false;
    }

    /* Initialize player position (center of screen) */
    game->player.x = (WINDOW_WIDTH - SPRITE_WIDTH) / 2.0f;
    game->player.y = (WINDOW_HEIGHT - SPRITE_HEIGHT) / 2.0f;
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
    if (game->player_texture) {
        SDL_DestroyTexture(game->player_texture);
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
                if (event.key.keysym.sym == SDLK_ESCAPE) {
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

    /* Check arrow keys and set velocity */
    if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) {
        game->player.vel_y = -SPRITE_SPEED;
    }
    if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) {
        game->player.vel_y = SPRITE_SPEED;
    }
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        game->player.vel_x = -SPRITE_SPEED;
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
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
    /* Set background color (grass green) and clear screen */
    SDL_SetRenderDrawColor(game->renderer,
                           COLOR_GRASS_R, COLOR_GRASS_G, COLOR_GRASS_B, 255);
    SDL_RenderClear(game->renderer);

    /* Define where to draw the player sprite */
    SDL_Rect dest_rect = {
        (int)game->player.x,
        (int)game->player.y,
        game->player.width,
        game->player.height
    };

    /*
     * SDL_RenderCopy draws a texture to the renderer
     * Parameters: renderer, texture, source rect (NULL = whole texture),
     *             destination rect (position and size on screen)
     */
    SDL_RenderCopy(game->renderer, game->player_texture, NULL, &dest_rect);

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

    /* Main game loop */
    while (game.running) {
        /* Calculate delta time (time since last frame in seconds) */
        Uint32 current_time = SDL_GetTicks();
        float delta_time = (current_time - last_time) / 1000.0f;
        last_time = current_time;

        /* Cap delta time to prevent huge jumps (e.g., after breakpoints) */
        if (delta_time > 0.1f) {
            delta_time = 0.1f;
        }

        /* Game loop phases */
        game_handle_events(&game);
        game_process_input(&game);
        game_update(&game, delta_time);
        game_render(&game);

        /*
         * Frame rate limiting
         * SDL_Delay gives CPU back to OS while waiting.
         * Note: VSYNC in the renderer also helps limit frame rate.
         */
        Uint32 frame_time = SDL_GetTicks() - current_time;
        if (frame_time < FRAME_DELAY) {
            SDL_Delay(FRAME_DELAY - frame_time);
        }
    }

    game_cleanup(&game);
    return 0;
}
