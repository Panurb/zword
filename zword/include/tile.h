#pragma once

#include <SFML/System.h>

#include "component.h"


typedef enum {
    TILE_ALTAR,
    TILE_BEACH,
    TILE_BEACH_CORNER,
    TILE_BOARD,
    TILE_BRICK,
    TILE_FENCE,
    TILE_GRASS,
    TILE_LEVEL_END,
    TILE_ROOF,
    TILE_SPAWNER,
    TILE_STONE,
    TILE_TILES,
    TILE_WATER,
    TILE_WOOD
} Tile;


void create_ground(Vector2f position, float width, float height, sfTexture* noise_texture);

void create_water(Vector2f position, float width, float height, sfTexture* noise_texture);

void create_beach(Vector2f position, float length, float angle);

void create_beach_corner(Vector2f position, float angle);

int create_wall(Vector2f pos, float angle, float width, float height, Filename filename);

void create_fence(Vector2f pos, float angle, float width, float height);

void create_glass(Vector2f pos, float angle);

int create_floor(Vector2f pos, float width, float height, float angle, Filename filename);

void create_roof(Vector2f pos, float width, float height, float angle);

void create_tile(Tile tile, Vector2f position, float angle, float width, float height);
