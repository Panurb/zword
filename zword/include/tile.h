#pragma once

#include <SFML/System.h>

#include "component.h"


typedef enum Tile {
    TILE_ALTAR,
    TILE_BEACH,
    TILE_BEACH_CORNER,
    TILE_BOARD,
    TILE_BRICK,
    TILE_GRASS,
    TILE_ROOF,
    TILE_STONE,
    TILE_TILES,
    TILE_WATER,
    TILE_WOOD
} Tile;


void create_ground(ComponentData* components, sfVector2f position, float width, float height, sfTexture* noise_texture);

void create_water(ComponentData* components, sfVector2f position, float width, float height, sfTexture* noise_texture);

void create_beach(ComponentData* components, sfVector2f position, float length, float angle);

void create_beach_corner(ComponentData* components, sfVector2f position, float angle);

int create_wall(ComponentData* components, sfVector2f pos, float angle, float width, float height, Filename filename);

void create_fence(ComponentData* components, sfVector2f pos, float angle, float width, float height);

void create_glass(ComponentData* components, sfVector2f pos, float angle);

int create_floor(ComponentData* components, sfVector2f pos, float width, float height, float angle, Filename filename);

void create_roof(ComponentData* components, sfVector2f pos, float width, float height, float angle);

void create_tile(ComponentData* components, Tile tile, sfVector2f position, float angle, float width, float height);
