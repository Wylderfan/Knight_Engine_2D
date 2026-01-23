/*
 * Knight Engine 2D - Camera System Implementation
 */

#include "graphics/camera.h"

void world_to_screen(const camera_t *camera, float world_x, float world_y,
                     int *screen_x, int *screen_y) {
    *screen_x = (int)(world_x - camera->x);
    *screen_y = (int)(world_y - camera->y);
}
