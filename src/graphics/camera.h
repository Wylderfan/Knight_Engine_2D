/*
 * Knight Engine 2D - Camera System
 *
 * Manages world vs screen coordinate conversion.
 *
 * Coordinate Systems:
 * - World coordinates: Position in the game world (can extend beyond screen)
 * - Screen coordinates: Position on the visible display (0,0 = top-left)
 *
 * The camera defines which portion of the world is visible on screen.
 * Objects at world position (x, y) appear at screen position (x - camera.x, y - camera.y)
 */

#pragma once

/*
 * Camera structure - defines the visible area of the world
 */
typedef struct {
    float x;  /* World x position of camera's top-left corner */
    float y;  /* World y position of camera's top-left corner */
} camera_t;

/*
 * Convert world coordinates to screen coordinates
 */
void world_to_screen(const camera_t *camera, float world_x, float world_y,
                     int *screen_x, int *screen_y);
