/*
 * Knight Engine 2D - Tile System Implementation
 */

#include "graphics/tile.h"
#include <string.h>

void tile_def_init(tile_def_t *def, int col, int row, bool walkable, const char *name) {
    def->tileset_col = col;
    def->tileset_row = row;
    def->is_walkable = walkable;

    if (name) {
        strncpy(def->name, name, TILE_NAME_MAX_LEN - 1);
        def->name[TILE_NAME_MAX_LEN - 1] = '\0';
    } else {
        def->name[0] = '\0';
    }
}

void tile_init_empty(tile_t *tile, tile_layer_e layer) {
    tile->tile_id = -1;
    tile->layer = layer;
}

void tile_init(tile_t *tile, int tile_id, tile_layer_e layer) {
    tile->tile_id = tile_id;
    tile->layer = layer;
}

void tile_def_get_src_rect(const tile_def_t *def, int *x, int *y, int *w, int *h) {
    *x = def->tileset_col * TILE_SIZE;
    *y = def->tileset_row * TILE_SIZE;
    *w = TILE_SIZE;
    *h = TILE_SIZE;
}

const char *tile_layer_name(tile_layer_e layer) {
    switch (layer) {
        case TILE_LAYER_GROUND:     return "ground";
        case TILE_LAYER_PATHS:      return "paths";
        case TILE_LAYER_DECORATION: return "decoration";
        case TILE_LAYER_OBJECTS:    return "objects";
        case TILE_LAYER_COLLISION:  return "collision";
        default:                    return "unknown";
    }
}
