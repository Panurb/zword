#include <string.h>
#include <stdio.h>

#include <SFML/System.h>

#include "tile.h"
#include "component.h"
#include "navigation.h"
#include "particle.h"
#include "enemy.h"


void create_ground(ComponentData* components, sfVector2f position, float width, float height, sfTexture* noise_texture) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0);
    ImageComponent_add(components, i, "grass_tile", width, height, LAYER_GROUND);

    if (noise_texture) {
        i = create_entity(components);
        CoordinateComponent_add(components, i, position, 0.0);
        ImageComponent* image = ImageComponent_add(components, i, "", 16.0, 16.0, LAYER_GROUND);
        image->scale = (sfVector2f) { 8.0, 8.0 };
        image->texture_changed = false;
        image->alpha = 1.0f;
        sfSprite_setTexture(image->sprite, noise_texture, false);
    }
}


void create_water(ComponentData* components, sfVector2f position, float width, float height, sfTexture* noise_texture) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ImageComponent_add(components, i, "water_tile", width, height, LAYER_GROUND);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_BARRIERS);

    if (noise_texture) {
        i = create_entity(components);
        CoordinateComponent_add(components, i, position, 0.0);
        ImageComponent* image = ImageComponent_add(components, i, "", 16.0, 16.0, LAYER_GROUND);
        image->scale = (sfVector2f) { 8.0f, 8.0f };
        image->texture_changed = false;
        image->alpha = 0.3f;
        sfSprite_setTexture(image->sprite, noise_texture, false);
    }
}


void create_beach(ComponentData* components, sfVector2f position, float length, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "beach_tile", 16.0f, length, LAYER_ROADS);
}


void create_beach_corner(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "beach_corner", 0, 0, LAYER_ROADS);
}


int create_wall(ComponentData* components, sfVector2f pos, float angle, float width, float height, Filename filename) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_WALLS);
    ImageComponent_add(components, i, filename, width, height, LAYER_WALLS);

    if (strcmp(filename, "wood_tile") == 0) {
        ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
        SoundComponent_add(components, i, "wood_hit");
    } else {
        ParticleComponent_add_type(components, i, PARTICLE_ROCK, 0.0f);
        SoundComponent_add(components, i, "stone_hit");
    }

    return i;
}


void create_fence(ComponentData* components, sfVector2f pos, float angle, float width, float height) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_BARRIERS);
    ImageComponent_add(components, i, "wood_tile", width, height, LAYER_WALLS);
}


void create_glass(ComponentData* components, sfVector2f pos, float angle) {
    create_fence(components, pos, angle, 1.0f, 4.0f);
    create_waypoint(components, sum(pos, polar_to_cartesian(1.5f, angle)));
    create_waypoint(components, diff(pos, polar_to_cartesian(1.5f, angle)));
}


int create_floor(ComponentData* components, sfVector2f pos, float width, float height, float angle, Filename filename) {
    printf("%f, %f, %f, %f\n", pos.x, pos.y, width, height);
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, angle);
    ImageComponent_add(components, i, filename, width, height, LAYER_FLOOR);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_FLOORS);

    return i;
}


void create_roof(ComponentData* components, sfVector2f pos, float width, float height, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_FLOORS);
    ImageComponent_add(components, i, "roof_tile", width, height, LAYER_ROOFS);
}


void create_tile(ComponentData* components, Tile tile, sfVector2f position, float angle, float width, float height) {
    switch (tile) {
        case TILE_ALTAR:
            create_wall(components, position, angle, width, height, "altar_tile");
            break;
        case TILE_BEACH:
            create_beach(components, position, max(width, height), angle);
            break;
        case TILE_BEACH_CORNER:
            create_beach_corner(components, position, angle);
            break;
        case TILE_BOARD:
            create_floor(components, position, width, height, angle, "board_tile");
            break;
        case TILE_BRICK:
            create_wall(components, position, angle, width, height, "brick_tile");
            break;
        case TILE_GRASS:
            create_ground(components, position, width, height, NULL);
            break;
        case TILE_ROOF:
            create_roof(components, position, width, height, angle);
            break;
        case TILE_SPAWNER:
            create_spawner(components, position, angle, width, height);
            break;
        case TILE_STONE:
            create_wall(components, position, angle, width, height, "stone_tile");
            break;
        case TILE_TILES:
            create_floor(components, position, width, height, angle, "tiles_tile");
            break;
        case TILE_WATER:
            create_water(components, position, width, height, NULL);
            break;
        case TILE_WOOD:
            create_wall(components, position, angle, width, height, "wood_tile");
            break;
    }
}
