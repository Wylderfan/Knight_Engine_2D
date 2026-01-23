/*
 * Knight Engine 2D - Input System
 *
 * Keyboard state tracking with edge detection.
 * Enables detection of key press/release edges, not just held state.
 */

#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "core/config.h"

/*
 * Input state structure - tracks current and previous frame key states
 */
typedef struct {
    const Uint8 *current;           /* Pointer to SDL's keyboard state */
    Uint8 previous[INPUT_MAX_KEYS]; /* Copy of last frame's state */
    int num_keys;                   /* Number of keys tracked */
} input_state_t;

/*
 * Initialize the input system
 */
void input_init(input_state_t *input);

/*
 * Update input state - call once per frame before processing input
 * Copies current state to previous, then SDL updates current automatically.
 */
void input_update(input_state_t *input);

/*
 * Check if a key is currently held down
 */
bool input_key_down(const input_state_t *input, SDL_Scancode key);

/*
 * Check if a key was just pressed this frame (down now, not down before)
 */
bool input_key_pressed(const input_state_t *input, SDL_Scancode key);

/*
 * Check if a key was just released this frame (not down now, was down before)
 */
bool input_key_released(const input_state_t *input, SDL_Scancode key);
