/*
 * Knight Engine 2D - Input System Implementation
 */

#include "input/input.h"
#include <string.h>

void input_init(input_state_t *input) {
    input->current = SDL_GetKeyboardState(&input->num_keys);
    memset(input->previous, 0, sizeof(input->previous));
}

void input_update(input_state_t *input) {
    /* Copy current state to previous before SDL updates it */
    for (int i = 0; i < input->num_keys && i < INPUT_MAX_KEYS; i++) {
        input->previous[i] = input->current[i];
    }
    /* SDL_GetKeyboardState pointer stays valid, SDL updates it internally */
}

bool input_key_down(const input_state_t *input, SDL_Scancode key) {
    return input->current[key] != 0;
}

bool input_key_pressed(const input_state_t *input, SDL_Scancode key) {
    return input->current[key] && !input->previous[key];
}

bool input_key_released(const input_state_t *input, SDL_Scancode key) {
    return !input->current[key] && input->previous[key];
}
