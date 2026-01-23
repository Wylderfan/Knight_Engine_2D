/*
 * Knight Engine 2D - Timer Utilities
 *
 * FPS tracking and time management utilities.
 */

#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "core/config.h"

/*
 * FPS counter state - tracks frame rate over time
 */
typedef struct {
    Uint32 last_time;    /* Last time FPS was calculated */
    int frame_count;     /* Frames since last calculation */
    float current_fps;   /* Most recent calculated FPS */
} fps_counter_t;

/*
 * Initialize FPS counter
 */
void fps_counter_init(fps_counter_t *fps);

/*
 * Update FPS counter - call once per frame
 * Returns true if FPS was recalculated this frame
 */
bool fps_counter_update(fps_counter_t *fps, Uint32 current_time);

/*
 * Get current FPS value
 */
float fps_counter_get(const fps_counter_t *fps);
