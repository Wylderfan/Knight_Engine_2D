/*
 * Knight Engine 2D - Timer Utilities Implementation
 */

#include "util/timer.h"
#include <stdbool.h>

void fps_counter_init(fps_counter_t *fps) {
    fps->last_time = SDL_GetTicks();
    fps->frame_count = 0;
    fps->current_fps = 0.0f;
}

bool fps_counter_update(fps_counter_t *fps, Uint32 current_time) {
    fps->frame_count++;

    if (current_time - fps->last_time >= FPS_UPDATE_INTERVAL) {
        fps->current_fps = fps->frame_count * 1000.0f / (current_time - fps->last_time);
        fps->frame_count = 0;
        fps->last_time = current_time;
        return true;
    }

    return false;
}

float fps_counter_get(const fps_counter_t *fps) {
    return fps->current_fps;
}
