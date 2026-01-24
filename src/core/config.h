/*
 * Knight Engine 2D - Configuration Constants
 *
 * Central location for all engine configuration values.
 * Adjust these to customize engine behavior.
 */

#pragma once

/* ============================================================================
 * WINDOW SETTINGS
 * ============================================================================ */

#define WINDOW_TITLE  "Knight Engine 2D - Sprite Demo"
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

/* ============================================================================
 * SPRITE SETTINGS
 * ============================================================================ */

#define SPRITE_WIDTH  64
#define SPRITE_HEIGHT 64
#define SPRITE_SPEED  300.0f  /* Pixels per second */

/* Player starting position (center of screen if not specified) */
#define PLAYER_START_X ((WINDOW_WIDTH - SPRITE_WIDTH) / 2.0f)
#define PLAYER_START_Y ((WINDOW_HEIGHT - SPRITE_HEIGHT) / 2.0f)

/* Maximum sprites in scene */
#define SPRITE_MAX_COUNT 256

/* ============================================================================
 * FRAME RATE & TIMING
 * ============================================================================ */

#define TARGET_FPS        60
#define FIXED_TIMESTEP    (1.0f / TARGET_FPS)  /* Fixed update rate for physics */
#define MAX_DELTA_TIME    0.1f   /* Cap delta time to prevent large jumps */
#define MAX_ACCUMULATOR   0.25f  /* Prevent spiral of death on slow frames */

/* FPS counter settings */
#define FPS_UPDATE_INTERVAL 500  /* Update FPS display every N milliseconds */
#define FPS_DISPLAY_ENABLED 1    /* Set to 0 to disable FPS in window title */
#define FPS_DEBUG_LOG       0    /* Set to 1 to log FPS vs target to console */

/* ============================================================================
 * CAMERA SETTINGS
 * ============================================================================ */

#define CAMERA_SPEED 200.0f

/* ============================================================================
 * TEXTURE SETTINGS
 * ============================================================================ */

#define TEXTURE_MAX_ENTRIES  32
#define TEXTURE_PATH_MAX_LEN 128

/* ============================================================================
 * ASSET PATHS
 * ============================================================================ */

#define PLAYER_TEXTURE_PATH "assets/player.png"
#define BACKGROUND_TEXTURE_PATH "assets/background.png"

/* ============================================================================
 * COLORS (RGB)
 * ============================================================================ */

/* Background color - grass green */
#define COLOR_BG_R 34
#define COLOR_BG_G 139
#define COLOR_BG_B 34

/* Player sprite fallback color - royal blue */
#define COLOR_PLAYER_R 65
#define COLOR_PLAYER_G 105
#define COLOR_PLAYER_B 225

/* ============================================================================
 * DEBUG SETTINGS
 * ============================================================================ */

#define DEBUG_OUTPUT_INTERVAL 500  /* Milliseconds between debug prints */

/* ============================================================================
 * INPUT SETTINGS
 * ============================================================================ */

#define INPUT_MAX_KEYS 512  /* SDL scancodes fit in this range */

/* ============================================================================
 * TILE SETTINGS
 * ============================================================================ */

#define TILE_SIZE             128   /* Tile dimensions in pixels (128x128) */
#define TILE_MAX_DEFINITIONS  256   /* Maximum tile types in a tileset */
#define TILE_MAX_LAYERS       8     /* Maximum layers in a tilemap */
#define TILE_NAME_MAX_LEN     32    /* Maximum length for tile type names */

/* ============================================================================
 * STRESS TEST SETTINGS (can be removed when not needed)
 * ============================================================================ */

#define STRESS_TEST_SPRITE_COUNT 150
