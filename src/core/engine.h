/*
 * Knight Engine 2D - Engine
 *
 * Core engine functions for initialization, cleanup, and the main game loop.
 */

#pragma once

#include <stdbool.h>

/* Forward declaration */
typedef struct game_state_t game_state_t;

/*
 * Initialize the game engine
 * Sets up renderer, textures, input, and initial game state.
 * Returns true on success, false on failure.
 */
bool engine_init(game_state_t *game);

/*
 * Clean up all engine resources
 * Destroys textures, renderer, and SDL subsystems.
 */
void engine_cleanup(game_state_t *game);

/*
 * Run the main game loop
 * Handles events, input, updates, and rendering until game exits.
 */
void engine_run(game_state_t *game);
