/*
 * Knight Engine 2D - Game State
 *
 * Central game state structure holding all game resources and data.
 */

#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "core/config.h"
#include "graphics/camera.h"
#include "graphics/renderer.h"
#include "graphics/sprite.h"
#include "graphics/texture.h"
#include "input/input.h"

/*
 * Game state structure - holds all game resources and data
 */
typedef struct {
    renderer_t renderer;
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
