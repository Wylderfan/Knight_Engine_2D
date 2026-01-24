/*
 * Knight Engine 2D - Tile System
 *
 * Data structures for tile-based maps and tilesets.
 * Uses a two-level system:
 *   - tile_def_t: Defines tile types with properties (stored in tileset)
 *   - tile_t: Map data referencing tile definitions by ID
 */

#pragma once

#include <stdbool.h>
#include "core/config.h"

/*
 * Tile layer enum - defines render and logic layers for tilemaps
 * Layers are rendered in order (GROUND first, then PATHS, etc.)
 */
typedef enum {
    TILE_LAYER_GROUND = 0,    /* Base terrain (grass, dirt, stone) */
    TILE_LAYER_PATHS,         /* Paths, roads, tilled soil */
    TILE_LAYER_DECORATION,    /* Flowers, rocks, debris */
    TILE_LAYER_OBJECTS,       /* Fences, buildings (if tile-based) */
    TILE_LAYER_COLLISION,     /* Collision-only layer (invisible) */
    TILE_LAYER_COUNT          /* Always last - used for iteration */
} tile_layer_e;

/*
 * Tile definition - properties of a tile type in a tileset
 * Stored in tileset, referenced by tile_t via tile_id.
 *
 * tileset_col/row: Position in tileset atlas (tile-index, not pixels)
 *                  Pixel coords calculated as: col * TILE_SIZE, row * TILE_SIZE
 * is_walkable:     Whether entities can traverse this tile
 * name:            Human-readable identifier for debugging/tools
 */
typedef struct {
    int tileset_col;                  /* Column in tileset atlas (0-based) */
    int tileset_row;                  /* Row in tileset atlas (0-based) */
    bool is_walkable;                 /* Can entities walk on this tile? */
    char name[TILE_NAME_MAX_LEN];     /* Tile type name ("grass", "water", etc.) */
} tile_def_t;

/*
 * Tile instance - a single tile in a map layer
 * References a tile_def_t by ID for its properties and visuals.
 *
 * tile_id: Index into tile definition array (-1 = empty/no tile)
 * layer:   Which layer this tile belongs to
 */
typedef struct {
    int tile_id;              /* Index into tile_def_t array (-1 = empty) */
    tile_layer_e layer;       /* Layer this tile is on */
} tile_t;

/*
 * Initialize a tile definition with default values
 */
void tile_def_init(tile_def_t *def, int col, int row, bool walkable, const char *name);

/*
 * Initialize an empty tile
 */
void tile_init_empty(tile_t *tile, tile_layer_e layer);

/*
 * Initialize a tile with a definition ID
 */
void tile_init(tile_t *tile, int tile_id, tile_layer_e layer);

/*
 * Get pixel coordinates for a tile definition's position in the tileset atlas
 * Returns source rectangle for rendering from tileset texture.
 */
void tile_def_get_src_rect(const tile_def_t *def, int *x, int *y, int *w, int *h);

/*
 * Get layer name as string (for debugging)
 */
const char *tile_layer_name(tile_layer_e layer);
