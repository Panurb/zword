#include <string.h>
#include <stdio.h>

#include "tile.h"
#include "component.h"
#include "navigation.h"
#include "particle.h"
#include "enemy.h"


void create_ground(Vector2f position, float width, float height) {
    int i = create_entity();
    CoordinateComponent_add(i, position, 0.0);
    ImageComponent_add(i, "grass_tile", width, height, LAYER_GROUND);

    // if (noise_texture) {
    //     i = create_entity();
    //     CoordinateComponent_add(i, position, 0.0);
    //     ImageComponent* image = ImageComponent_add(i, "", 16.0, 16.0, LAYER_GROUND);
    //     image->scale = (Vector2f) { 8.0, 8.0 };
    //     image->texture_changed = false;
    //     image->alpha = 1.0f;
    //     // sfSprite_setTexture(image->sprite, noise_texture, false);
    // }
}


void create_water(Vector2f position, float width, float height) {
    int i = create_entity();
    CoordinateComponent_add(i, position, 0.0f);
    ImageComponent_add(i, "water_tile", width, height, LAYER_GROUND);
    ColliderComponent_add_rectangle(i, width, height, GROUP_WALLS);

    // if (noise_texture) {
    //     i = create_entity();
    //     CoordinateComponent_add(i, position, 0.0);
    //     ImageComponent* image = ImageComponent_add(i, "", 16.0, 16.0, LAYER_GROUND);
    //     image->scale = (Vector2f) { 8.0f, 8.0f };
    //     image->texture_changed = false;
    //     image->alpha = 0.3f;
    //     // sfSprite_setTexture(image->sprite, noise_texture, false);
    // }
}


void create_beach(Vector2f position, float length, float angle) {
    int i = create_entity();
    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "beach_tile", 16.0f, length, LAYER_PATHS);
}


void create_beach_corner(Vector2f position, float angle) {
    int i = create_entity();
    CoordinateComponent_add(i, position, angle);
    ImageComponent_add(i, "beach_corner", 0, 0, LAYER_PATHS);
}


int create_wall(Vector2f pos, float angle, float width, float height, Filename filename) {
    int i = create_entity();
    CoordinateComponent_add(i, pos, angle);
    ColliderComponent_add_rectangle(i, width, height, GROUP_WALLS);
    ImageComponent_add(i, filename, width, height, LAYER_WALLS);

    if (strcmp(filename, "wood_tile") == 0) {
        ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
        SoundComponent_add(i, "wood_hit");
    } else {
        ParticleComponent_add_type(i, PARTICLE_ROCK, 0.0f);
        SoundComponent_add(i, "stone_hit");
    }

    return i;
}


void create_fence(Vector2f pos, float angle, float width, float height) {
    int i = create_entity();
    CoordinateComponent_add(i, pos, angle);
    ColliderComponent_add_rectangle(i, width, height, GROUP_BARRIERS);
    ImageComponent_add(i, "fence_tile", width, height, LAYER_WALLS);
}


void create_glass(Vector2f pos, float angle) {
    create_fence(pos, angle, 1.0f, 4.0f);
    create_waypoint(sum(pos, polar_to_cartesian(1.5f, angle)));
    create_waypoint(diff(pos, polar_to_cartesian(1.5f, angle)));
}


int create_floor(Vector2f pos, float width, float height, float angle, Filename filename) {
    printf("%f, %f, %f, %f\n", pos.x, pos.y, width, height);
    int i = create_entity();

    CoordinateComponent_add(i, pos, angle);
    ImageComponent_add(i, filename, width, height, LAYER_FLOOR);
    ColliderComponent_add_rectangle(i, width, height, GROUP_FLOORS);

    return i;
}


void create_roof(Vector2f pos, float width, float height, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, pos, angle);
    ColliderComponent_add_rectangle(i, width, height, GROUP_FLOORS);
    ImageComponent_add(i, "roof_tile", width, height, LAYER_ROOFS);
}


void create_tile(Tile tile, Vector2f position, float angle, float width, float height) {
    switch (tile) {
        case TILE_ALTAR:
            create_wall(position, angle, width, height, "altar_tile");
            break;
        case TILE_BEACH:
            create_beach(position, maxi(width, height), angle);
            break;
        case TILE_BEACH_CORNER:
            create_beach_corner(position, angle);
            break;
        case TILE_BOARD:
            create_floor(position, width, height, angle, "board_tile");
            break;
        case TILE_BRICK:
            create_wall(position, angle, width, height, "brick_tile");
            break;
        case TILE_FENCE:
            create_fence(position, angle, width, height);
            break;
        case TILE_GRASS:
            create_ground(position, width, height);
            break;
        case TILE_LEVEL_END:
            create_level_end(position, angle, width, height);
            break;
        case TILE_ROOF:
            create_roof(position, width, height, angle);
            break;
        case TILE_SPAWNER:
            create_spawner(position, angle, width, height);
            break;
        case TILE_STONE:
            create_wall(position, angle, width, height, "stone_tile");
            break;
        case TILE_TILES:
            create_floor(position, width, height, angle, "tiles_tile");
            break;
        case TILE_WATER:
            create_water(position, width, height);
            break;
        case TILE_WOOD:
            create_wall(position, angle, width, height, "wood_tile");
            break;
    }
}
