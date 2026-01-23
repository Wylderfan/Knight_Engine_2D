/*
 * Knight Engine 2D - Input Configuration
 *
 * Key bindings for game controls.
 * Uses SDL_SCANCODE_* values (physical key positions).
 */

#pragma once

#include <SDL2/SDL.h>

/* ============================================================================
 * MOVEMENT KEYS
 * ============================================================================ */

/* Primary movement (Arrow keys) */
#define KEY_MOVE_UP     SDL_SCANCODE_UP
#define KEY_MOVE_DOWN   SDL_SCANCODE_DOWN
#define KEY_MOVE_LEFT   SDL_SCANCODE_LEFT
#define KEY_MOVE_RIGHT  SDL_SCANCODE_RIGHT

/* Alternate movement (WASD) */
#define KEY_MOVE_UP_ALT     SDL_SCANCODE_W
#define KEY_MOVE_DOWN_ALT   SDL_SCANCODE_S
#define KEY_MOVE_LEFT_ALT   SDL_SCANCODE_A
#define KEY_MOVE_RIGHT_ALT  SDL_SCANCODE_D

/* ============================================================================
 * CAMERA KEYS
 * ============================================================================ */

#define KEY_CAM_UP    SDL_SCANCODE_I
#define KEY_CAM_DOWN  SDL_SCANCODE_K
#define KEY_CAM_LEFT  SDL_SCANCODE_J
#define KEY_CAM_RIGHT SDL_SCANCODE_L

/* ============================================================================
 * SYSTEM KEYS
 * ============================================================================ */

/* Menu/quit keys (uses SDLK_* for key symbols) */
#define KEY_QUIT     SDLK_ESCAPE
#define KEY_QUIT_ALT SDLK_q

/* Debug toggle */
#define KEY_DEBUG_TOGGLE SDL_SCANCODE_P

/* STRESS_TEST - Toggle key */
#define KEY_STRESS_TEST SDL_SCANCODE_T
