/*
 * Knight Engine 2D - Game Logic
 *
 * Core game logic including input processing and state updates.
 */

#pragma once

/* Forward declaration */
typedef struct game_state_t game_state_t;

/*
 * Process continuous input (held keys)
 * Updates player velocity based on movement keys.
 */
void game_process_input(game_state_t *game);

/*
 * Update game logic
 * Handles debug toggle, stress test, camera movement, sprite updates,
 * and player position clamping.
 */
void game_update(game_state_t *game, float delta_time);
