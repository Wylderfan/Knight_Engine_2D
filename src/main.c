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

/* ============================================================================
 * KEY BINDINGS - Will move to input/input_config.h in Phase 2
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
#define KEY_QUIT     SDLK_ESCAPE
#define KEY_QUIT_ALT SDLK_q

/* Camera movement keys (IJKL) */
#define KEY_CAM_UP    SDL_SCANCODE_I
#define KEY_CAM_DOWN  SDL_SCANCODE_K
#define KEY_CAM_LEFT  SDL_SCANCODE_J
#define KEY_CAM_RIGHT SDL_SCANCODE_L

/* Debug key */
#define KEY_DEBUG_TOGGLE SDL_SCANCODE_P

/* STRESS_TEST - Toggle key */
#define KEY_STRESS_TEST SDL_SCANCODE_T

/* ============================================================================
 * INPUT SYSTEM - Keyboard state tracking with edge detection
 * ============================================================================ */

/*
 * Input state structure - tracks current and previous frame key states
 * Enables detection of key press/release edges, not just held state.
 */
typedef struct {
    const Uint8 *current;           /* Pointer to SDL's keyboard state */
    Uint8 previous[INPUT_MAX_KEYS]; /* Copy of last frame's state */
    int num_keys;                   /* Number of keys tracked */
} input_state_t;

/*
 * Initialize the input system
 */
static void input_init(input_state_t *input) {
    input->current = SDL_GetKeyboardState(&input->num_keys);
    memset(input->previous, 0, sizeof(input->previous));
}

/*
 * Update input state - call once per frame before processing input
 * Copies current state to previous, then SDL updates current automatically.
 */
static void input_update(input_state_t *input) {
    /* Copy current state to previous before SDL updates it */
    for (int i = 0; i < input->num_keys && i < INPUT_MAX_KEYS; i++) {
        input->previous[i] = input->current[i];
    }
    /* SDL_GetKeyboardState pointer stays valid, SDL updates it internally */
}

/*
 * Check if a key is currently held down
 */
static bool input_key_down(const input_state_t *input, SDL_Scancode key) {
    return input->current[key] != 0;
}

/*
 * Check if a key was just pressed this frame (down now, not down before)
 */
static bool input_key_pressed(const input_state_t *input, SDL_Scancode key) {
    return input->current[key] && !input->previous[key];
}

/*
 * Check if a key was just released this frame (not down now, was down before)
 */
static bool input_key_released(const input_state_t *input, SDL_Scancode key) {
    return !input->current[key] && input->previous[key];
}

/* ============================================================================
 * CAMERA SYSTEM - World vs screen coordinate management
 * ============================================================================
 *
 * Coordinate Systems:
 * - World coordinates: Position in the game world (can extend beyond screen)
 * - Screen coordinates: Position on the visible display (0,0 = top-left)
 *
 * The camera defines which portion of the world is visible on screen.
 * Objects at world position (x, y) appear at screen position (x - camera.x, y - camera.y)
 */

/*
 * Camera structure - defines the visible area of the world
 */
typedef struct {
    float x;  /* World x position of camera's top-left corner */
    float y;  /* World y position of camera's top-left corner */
} camera_t;

/*
 * Convert world coordinates to screen coordinates
 */
static void world_to_screen(const camera_t *camera, float world_x, float world_y,
                            int *screen_x, int *screen_y) {
    *screen_x = (int)(world_x - camera->x);
    *screen_y = (int)(world_y - camera->y);
}

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
    int z_index;          /* Render order: 0 = background, 50 = entities, 100 = UI */
    double angle;         /* Rotation in degrees (clockwise) */
    SDL_RendererFlip flip; /* SDL_FLIP_NONE, SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL */
    SDL_Texture *texture;
    /* Debug visualization */
    bool show_debug_bounds;  /* Draw bounding box when debug mode is on */
    Uint8 debug_r, debug_g, debug_b;  /* Debug border color */
} sprite_t;

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
 * Prevents loading the same texture multiple times and handles cleanup.
 */
typedef struct {
    texture_entry_t entries[TEXTURE_MAX_ENTRIES];
    int count;
    SDL_Renderer *renderer;
} texture_manager_t;

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

/*
 * Render a sprite to the screen
 * Call this for each sprite during the render phase.
 *
 * camera:   Camera for world-to-screen coordinate conversion.
 * src_rect: Optional source rectangle for sprite sheets.
 *           Pass NULL to render the entire texture.
 *           When non-NULL, specifies which portion of the texture to render.
 */
static void sprite_render(SDL_Renderer *renderer, const sprite_t *sprite,
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

/*
 * Render a sprite with extended options (rotation, flip, color modulation)
 *
 * camera:   Camera for world-to-screen coordinate conversion.
 * src_rect:  Optional source rectangle for sprite sheets (NULL = full texture)
 * angle:     Rotation in degrees (clockwise)
 * center:    Point to rotate around (NULL = center of sprite)
 * flip:      SDL_FLIP_NONE, SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL, or combined
 * r, g, b:   Color modulation (255 = no change, lower = tint toward that color)
 */
static void sprite_render_ex(SDL_Renderer *renderer, const sprite_t *sprite,
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

/* ============================================================================
 * DEBUG UTILITIES - Visual debugging tools
 * ============================================================================ */

/*
 * Draw a colored rectangle outline (for collision boxes, debug bounds, etc.)
 * Uses world coordinates - converts to screen space using camera.
 */
static void debug_draw_rect(SDL_Renderer *renderer, const camera_t *camera,
                            float world_x, float world_y, int width, int height,
                            Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int screen_x, screen_y;
    world_to_screen(camera, world_x, world_y, &screen_x, &screen_y);

    SDL_Rect rect = { screen_x, screen_y, width, height };

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawRect(renderer, &rect);
}

/*
 * Draw a filled colored rectangle (for debug visualization)
 * Uses world coordinates - converts to screen space using camera.
 */
static void debug_fill_rect(SDL_Renderer *renderer, const camera_t *camera,
                            float world_x, float world_y, int width, int height,
                            Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int screen_x, screen_y;
    world_to_screen(camera, world_x, world_y, &screen_x, &screen_y);

    SDL_Rect rect = { screen_x, screen_y, width, height };

    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
}

/*
 * Draw a rotated rectangle outline (for debug bounds on rotated sprites)
 * Angle is in degrees (clockwise), rotation is around the center of the rect.
 */
static void debug_draw_rect_rotated(SDL_Renderer *renderer, const camera_t *camera,
                                    float world_x, float world_y, int width, int height,
                                    double angle, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    int screen_x, screen_y;
    world_to_screen(camera, world_x, world_y, &screen_x, &screen_y);

    /* Center of the rectangle */
    float cx = screen_x + width / 2.0f;
    float cy = screen_y + height / 2.0f;

    /* Convert angle to radians (SDL uses clockwise degrees) */
    double rad = angle * M_PI / 180.0;
    float cos_a = (float)cos(rad);
    float sin_a = (float)sin(rad);

    /* Four corners relative to center (before rotation) */
    float hw = width / 2.0f;
    float hh = height / 2.0f;

    /* Rotate each corner around center */
    SDL_Point corners[4];
    float offsets[4][2] = {
        { -hw, -hh },  /* Top-left */
        {  hw, -hh },  /* Top-right */
        {  hw,  hh },  /* Bottom-right */
        { -hw,  hh }   /* Bottom-left */
    };

    for (int i = 0; i < 4; i++) {
        float rx = offsets[i][0] * cos_a - offsets[i][1] * sin_a;
        float ry = offsets[i][0] * sin_a + offsets[i][1] * cos_a;
        corners[i].x = (int)(cx + rx);
        corners[i].y = (int)(cy + ry);
    }

    /* Draw lines between corners */
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderDrawLine(renderer, corners[0].x, corners[0].y, corners[1].x, corners[1].y);
    SDL_RenderDrawLine(renderer, corners[1].x, corners[1].y, corners[2].x, corners[2].y);
    SDL_RenderDrawLine(renderer, corners[2].x, corners[2].y, corners[3].x, corners[3].y);
    SDL_RenderDrawLine(renderer, corners[3].x, corners[3].y, corners[0].x, corners[0].y);
}

/* ============================================================================
 * TEXTURE MANAGER - Load, cache, and manage textures
 * ============================================================================ */

/*
 * Initialize the texture manager
 */
static void texture_manager_init(texture_manager_t *tm, SDL_Renderer *renderer) {
    tm->count = 0;
    tm->renderer = renderer;
    for (int i = 0; i < TEXTURE_MAX_ENTRIES; i++) {
        tm->entries[i].path[0] = '\0';
        tm->entries[i].texture = NULL;
        tm->entries[i].width = 0;
        tm->entries[i].height = 0;
    }
}

/*
 * Load a texture from file path
 * Returns the loaded texture, or NULL on failure.
 * Caches textures - subsequent loads of the same path return cached version.
 */
static SDL_Texture *texture_load(texture_manager_t *tm, const char *path) {
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

/*
 * Get a previously loaded texture by path
 * Returns NULL if not found (use texture_load to load first)
 */
static SDL_Texture *texture_get(texture_manager_t *tm, const char *path) {
    for (int i = 0; i < tm->count; i++) {
        if (strcmp(tm->entries[i].path, path) == 0) {
            return tm->entries[i].texture;
        }
    }
    return NULL;
}

/*
 * Get texture dimensions by path
 * Returns true if found, false otherwise
 */
static bool texture_get_size(texture_manager_t *tm, const char *path,
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

/*
 * Clean up all loaded textures
 */
static void texture_manager_cleanup(texture_manager_t *tm) {
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

/* ============================================================================
 * TEXTURE CREATION HELPERS
 * ============================================================================ */

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
            spr->texture = create_colored_texture(game->renderer, 32, 32,
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
        player->texture = create_colored_texture(game->renderer,
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
    test->texture = create_colored_texture(game->renderer,
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
        game->background = create_colored_texture(game->renderer,
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
