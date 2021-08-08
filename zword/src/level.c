#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "level.h"
#include "image.h"
#include "component.h"
#include "player.h"
#include "util.h"
#include "enemy.h"
#include "weapon.h"
#include "vehicle.h"
#include "navigation.h"
#include "perlin.h"
#include "item.h"
#include "particle.h"
#include "road.h"
#include "grid.h"
#include "collider.h"
#include "sound.h"
#include "door.h"


void create_wall(ComponentData* components, sfVector2f pos, float angle, float width, float height, Filename filename) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_WALLS);
    ImageComponent_add(components, i, filename, width, height, LAYER_WALLS);

    if (strcmp(filename, "wood_tile") == 0) {
        ParticleComponent_add_splinter(components, i);
        SoundComponent_add(components, i, "wood_hit");
    } else {
        ParticleComponent_add_rock(components, i);
        SoundComponent_add(components, i, "stone_hit");
    }
}


void create_fire(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(components, i, 5.0, 2.0 * M_PI, orange, 0.8, 10.0)->flicker = 0.2;
    ParticleComponent* particle = ParticleComponent_add(components, i, 0.5 * M_PI, 1.0, 0.6, 0.2, 1.0, 5.0, orange, orange);
    particle->loop = true;
    particle->enabled = true;
    ColliderComponent_add_circle(components, i, 0.35, GROUP_WALLS);
    ImageComponent_add(components, i, "fire", 1.0, 1.0, LAYER_PARTICLES);
}


void create_light(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, rand_angle());
    ParticleComponent_add_fire(components, i, 0.25f);
    ImageComponent_add(components, i, "candle", 1.0f, 1.0f, LAYER_ITEMS);
}


void create_floor(ComponentData* components, sfVector2f pos, float width, float height, float angle, Filename filename) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, angle);
    ImageComponent_add(components, i, filename, width, height, LAYER_FLOOR);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_FLOORS);
}


void create_roof(ComponentData* components, sfVector2f pos, float width, float height, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_FLOORS);
    ImageComponent_add(components, i, "roof_tile", width, height, LAYER_ROOFS);
}


void create_item(ComponentData* components, sfVector2f position, int tier) {
    switch (tier) {
        case 0:
            if (randi(0, 1) == 0) {
                create_flashlight(components, position);
            } else {
                create_gas(components, position);
            }
            break;
        case 1:
            if (randi(0, 1) == 0) {
                create_axe(components, position);
            } else {
                create_pistol(components, position);
                for (int i = 0; i < randi(1, 3); i++) {
                    create_ammo(components, sum(position, rand_vector()), AMMO_PISTOL);
                }
            }
            break;
        case 2:
            create_shotgun(components, position);
            for (int i = 0; i < randi(1, 3); i++) {
                    create_ammo(components, sum(position, rand_vector()), AMMO_SHOTGUN);
            }
            break;
        case 3:
            create_assault_rifle(components, position);
            for (int i = 0; i < randi(1, 3); i++) {
                create_ammo(components, sum(position, rand_vector()), AMMO_RIFLE);
            }
            break;
        default:
            create_ammo(components, position, rand() % 3 + 1);
            break;
    }
}


void create_bench(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "bench", 1.0f, 3.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 0.8f, 2.8f, GROUP_WALLS);
    ParticleComponent_add_splinter(components, i);
    HealthComponent_add(components, i, 200, "", "", "wood_destroy");
    PhysicsComponent_add(components, i, 2.0f);
    SoundComponent_add(components, i, "wood_hit");

    int j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {0.0f, -1.0f}, 0.0f);
    ImageComponent_add(components, j, "bench_debris", 1.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 0.8f, 1.2f, GROUP_DEBRIS);
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);

    j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {0.0f, 0.5f}, 0.0f);
    ImageComponent_add(components, j, "bench_destroyed", 1.0f, 2.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 0.8f, 1.4f, GROUP_DEBRIS);
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);
}


void create_table(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    ImageComponent_add(components, i, "table", 3.0f, 3.0f, LAYER_ITEMS)->alpha = 1.0f;
    ColliderComponent_add_circle(components, i, 1.4f, GROUP_WALLS);
    ParticleComponent_add_splinter(components, i);
    HealthComponent_add(components, i, 200, "", "", "wood_destroy");
    PhysicsComponent_add(components, i, 2.0f);
    SoundComponent_add(components, i, "wood_hit");

    int j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {-0.6f, 0.0f}, 0.0f);
    ImageComponent_add(components, j, "table_destroyed", 2.0f, 3.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 1.2f, 1.8f, GROUP_DEBRIS);
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);

    j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {0.6f, 0.0f}, M_PI);
    ImageComponent_add(components, j, "table_destroyed", 2.0f, 3.0f, LAYER_CORPSES)->alpha = 0.0f;
    ColliderComponent_add_rectangle(components, j, 1.2f, 1.8f, GROUP_DEBRIS);
    PhysicsComponent_add(components, j, 1.0f);
    add_child(components, i, j);
}


void create_hay_bale(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "hay_bale", 3.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 2.8f, 1.5f, GROUP_WALLS);
    ParticleComponent_add_splinter(components, i);
}


void create_stove(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "stove", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_sparks(components, i);
    SoundComponent_add(components, i, "metal_hit");
}


void create_sink(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "sink", 2.0f, 3.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.8f, 2.8f, GROUP_WALLS);
    ParticleComponent_add_sparks(components, i);
    SoundComponent_add(components, i, "metal_hit");
}


void create_toilet(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "toilet", 2.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 1.8f, 1.3f, GROUP_WALLS);
    ParticleComponent_add_sparks(components, i);
    SoundComponent_add(components, i, "metal_hit");
}


void create_bed(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "bed", 4.0f, 2.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 3.8f, 1.8f, GROUP_WALLS);
    ParticleComponent_add_splinter(components, i);
    SoundComponent_add(components, i, "wood_hit");
}


void create_lamp(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ImageComponent_add(components, i, "lamp", 1.0f, 1.0f, LAYER_ITEMS);
    ParticleComponent_add_splinter(components, i);
    SoundComponent_add(components, i, "wood_hit");
    LightComponent_add(components, i, 10.0f, 2.0f * M_PI, sfWhite, 0.5f, 5.0f)->flicker = 0.1f;
}


void create_decal(ComponentData* components, sfVector2f pos, float width, float height, Filename filename) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, rand_angle());
    ImageComponent_add(components, i, filename, width, height, LAYER_DECALS);
}


void create_church(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = polar_to_cartesian(1.0f, angle + 0.5f * M_PI);

    create_floor(components, pos, 30.0f, 10.0f, angle, "tiles_tile");
    create_floor(components, sum(pos, mult(5.0f, w)), 10.0f, 20.0f, angle, "tiles_tile");

    create_door(components, sum(pos, lin_comb(-14.5f, w, 1.0f, h)), angle - 0.5f * M_PI);
    create_door(components, sum(pos, lin_comb(-14.5f, w, -1.0f, h)), angle + 0.5f * M_PI);

    // Back wall
    create_wall(components, sum(pos, mult(14.5f, w)), angle + 0.5f * M_PI, 8.0f, 1.0f, "stone_tile");

    // Altar
    create_wall(components, sum(pos, mult(12.5f, w)), angle, 3.0f, 8.0f, "altar_tile");
    create_light(components, sum(pos, sum(mult(12.5f, w), mult(-3.0f, h))));
    create_light(components, sum(pos, sum(mult(12.5f, w), mult(3.0f, h))));
    int k = create_entity(components);
    CoordinateComponent_add(components, k, sum(pos, mult(12.5f, w)), rand_angle());
    ImageComponent_add(components, k, "blood_large", 2.0f, 2.0f, LAYER_ITEMS);

    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 7; i++) {
            create_bench(components, sum(pos, sum(mult((i - 5.75f) * 2.0f, w), mult(2.5f, h))), angle);
        }

        // Entrance
        create_wall(components, sum(pos, sum(mult(-14.5f, w), mult(3.0f, h))), angle + 0.5f * M_PI, 2.0f, 1.0f, "stone_tile");
        create_wall(components, sum(pos, sum(mult(-13.0f, w), mult(3.5f, h))), angle, 2.0f, 1.0f, "altar_tile");
        create_light(components, sum(pos, sum(mult(-13.0f, w), mult(3.5f, h))));

        // Side wall entrance
        create_wall(components, sum(pos, sum(mult(-6.0f, w), mult(4.5f, h))), angle, 18.0f, 1.0f, "stone_tile");

        // Side wall back
        create_wall(components, sum(pos, sum(mult(11.0f, w), mult(4.5f, h))), angle, 8.0f, 1.0f, "stone_tile");
        
        // Side thingies
        create_wall(components, sum(pos, sum(mult(5.0f, w), mult(9.5f, h))), angle, 10.0f, 1.0f, "stone_tile");
        create_wall(components, sum(pos, sum(mult(0.5f, w), mult(7.0f, h))), angle + 0.5f * M_PI, 4.0f, 1.0f, "stone_tile");
        create_wall(components, sum(pos, sum(mult(9.5f, w), mult(7.0f, h))), angle + 0.5f * M_PI, 4.0f, 1.0f, "stone_tile");
        create_wall(components, sum(pos, sum(mult(5.0f, w), mult(8.5f, h))), angle, 2.0f, 1.0f, "altar_tile");
        create_light(components, sum(pos, sum(mult(5.0f, w), mult(8.5f, h))));
        create_waypoint(components, sum(pos, sum(mult(5.0f, w), mult(5.0f, h))));

        // Outside
        create_waypoint(components, sum(pos, sum(mult(-16.0f, w), mult(6.0f, h))));
        create_waypoint(components, sum(pos, sum(mult(-8.0f, w), mult(6.0f, h))));
        create_waypoint(components, sum(pos, sum(mult(-1.0f, w), mult(11.0f, h))));
        create_waypoint(components, sum(pos, sum(mult(11.0f, w), mult(11.0f, h))));
        create_waypoint(components, sum(pos, sum(mult(16.0f, w), mult(6.0f, h))));

        h = mult(-1.0f, h);
    }

    create_waypoint(components, sum(pos, mult(-16.0f, w)));
    create_waypoint(components, sum(pos, mult(-12.0f, w)));
    create_waypoint(components, sum(pos, mult(2.0f, w)));
    create_waypoint(components, sum(pos, mult(8.0f, w)));

    int items = randi(1, 2);
    int locations[2] = {0, 1};
    permute(locations, 2);
    for (int i = 0; i < items; i++) {
        switch (locations[i]) {
            case 0:
                create_item(components, sum(pos, sum(mult(5.0f, w), mult(6.5f, h))), randi(2, 3));
                break;
            case 1:
                create_item(components, sum(pos, sum(mult(5.0f, w), mult(-6.5f, h))), randi(2, 3));
                break;
        }
    }
}


void create_barn(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = polar_to_cartesian(1.0f, angle + 0.5f * M_PI);

    create_floor(components, pos, 25.0f, 14.0f, angle, "board_tile");

    // Back wall
    create_wall(components, sum(pos, mult(12.0f, w)), angle + 0.5f * M_PI, 14.0f, 1.0f, "wood_tile");

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            sfVector2f r = sum(mult(3.0f * i - 4.5f, h), mult(6.5f + 2.0f * j, w));
            create_hay_bale(components, sum(pos, r), angle + 0.5f * M_PI + randf(-0.1f, 0.1f));
        }
    }

    for (int i = 0; i < 2; i++) {
        // Entrance
        create_wall(components, sum(pos, sum(mult(-12.0f, w), mult(4.5f, h))), angle + 0.5f * M_PI, 5.0f, 1.0f, "wood_tile");

        // Side
        create_wall(components, sum(pos, mult(6.5f, h)), angle, 23.0f, 1.0f, "wood_tile");

        // Middle pile
        create_hay_bale(components, sum(pos, sum(mult(-4.0f, w), mult(1.0f, h))), angle + randf(-0.1f, 0.1f));
        create_hay_bale(components, sum(pos, sum(mult(-1.0f, w), mult(1.0f, h))), angle + randf(-0.1f, 0.1f));
        create_waypoint(components, sum(pos, lin_comb(-6.5f, w, 3.0f, h)));
        create_waypoint(components, sum(pos, lin_comb(1.5f, w, 3.0f, h)));
        create_light(components, sum(pos, lin_comb(-1.0f, w, 1.0f, h)));
        create_light(components, sum(pos, lin_comb(-4.0f, w, 1.0f, h)));

        // Outside
        create_waypoint(components, sum(pos, lin_comb(-13.5f, w, 8.0f, h)));
        create_waypoint(components, sum(pos, lin_comb(0.0f, w, 8.0f, h)));
        create_waypoint(components, sum(pos, lin_comb(13.5f, w, 8.0f, h)));

        h = mult(-1.0f, h);
    }

    create_waypoint(components, sum(pos, mult(-11.0f, w)));
    create_waypoint(components, sum(pos, mult(-13.5f, w)));
    create_waypoint(components, sum(pos, mult(13.5f, w)));

    int items = randi(1, 3);
    int locations[3] = {0, 1, 2};
    permute(locations, 3);
    for (int i = 0; i < items; i++) {
        switch (locations[i]) {
            case 0:
                create_item(components, sum(pos, lin_comb(4.0f, w, 4.0f, h)), i);
                break;
            case 1:
                create_item(components, sum(pos, lin_comb(4.0f, w, -4.0f, h)), i);
                break;
            case 2:
                create_item(components, sum(pos, mult(4.0f, w)), i);
                break;
        }
    }
}


void create_house(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 16.0f, 16.0f, angle, "board_tile");
    // create_roof(components, sum(pos, mult(4.0f, h)), 16.0f, 8.0f, angle);
    // create_roof(components, sum(pos, mult(-4.0f, h)), 16.0f, 8.0f, angle + M_PI);

    // Outside walls
    create_wall(components, sum(pos, mult(-7.5f, w)), angle + 0.5 * M_PI, 16.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, mult(7.5f, h)), angle, 14.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, mult(-7.5f, h)), angle, 14.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(7.5f, w, 4.5f, h)), angle + 0.5 * M_PI, 7.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(7.5f, w, -4.5f, h)), angle + 0.5 * M_PI, 7.0f, 1.0f, "brick_tile");

    create_waypoint(components, sum(pos, lin_comb(9.0f, w, 9.0f, h)));
    create_waypoint(components, sum(pos, lin_comb(9.0f, w, -9.0f, h)));
    create_waypoint(components, sum(pos, lin_comb(-9.0f, w, 9.0f, h)));
    create_waypoint(components, sum(pos, lin_comb(-9.0f, w, -9.0f, h)));
    create_waypoint(components, sum(pos, lin_comb(-9.0f, w, 0.0f, h)));
    create_waypoint(components, sum(pos, lin_comb(0.0f, w, 9.0f, h)));
    create_waypoint(components, sum(pos, lin_comb(0.0f, w, -9.0f, h)));

    create_door(components, sum(pos, mult(7.5f, w)), angle + 0.5f * M_PI);

    // Toilet
    create_floor(components, sum(pos, lin_comb(-4.5f, w, 4.5f, h)), 6.0f, 6.0f, angle, "altar_tile");
    create_wall(components, sum(pos, lin_comb(-6.5f, w, 1.75f, h)), angle, 1.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, lin_comb(-1.75f, w, 4.5f, h)), angle + 0.5f * M_PI, 5.0f, 0.5f, "wood_tile");
    create_toilet(components, sum(pos, lin_comb(-6.0f, w, 6.0f, h)), angle + 1.5f * M_PI);
    create_sink(components, sum(pos, lin_comb(-3.5f, w, 6.0f, h)), angle + 1.5f * M_PI);

    // Bedroom
    create_wall(components, sum(pos, lin_comb(0.0f, w, 1.75f, h)), angle, 8.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, lin_comb(6.5f, w, 1.75f, h)), angle, 1.0f, 0.5f, "wood_tile");
    create_bed(components, sum(pos, lin_comb(-0.5f, w, 5.0f, h)), angle + 1.5f * M_PI);
    create_lamp(components, sum(pos, lin_comb(6.5f, w, 6.5f, h)));

    // Living room
    create_wall(components, sum(pos, lin_comb(0.0f, w, -1.75f, h)), angle, 4.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, lin_comb(-5.5f, w, -1.75f, h)), angle, 3.0f, 0.5f, "wood_tile");
    create_bench(components, sum(pos, lin_comb(-3.0f, w, -5.5f, h)), angle + M_PI);
    create_table(components, sum(pos, lin_comb(-5.5f, w, -5.5f, h)));
    create_lamp(components, sum(pos, lin_comb(-5.5f, w, -5.5f, h)));

    // Kitchen
    create_wall(components, sum(pos, lin_comb(5.5f, w, -1.75f, h)), angle, 3.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, lin_comb(0.0, w, -4.5f, h)), angle + 0.5f * M_PI, 5.0f, 0.5f, "wood_tile");
    create_stove(components, sum(pos, lin_comb(6.0f, w, -6.0f, h)), angle + M_PI);
    create_sink(components, sum(pos, lin_comb(6.0f, w, -3.5f, h)), angle + M_PI);

    create_waypoint(components, sum(pos, mult(9.0f, w)));
    create_waypoint(components, sum(pos, mult(5.0f, w)));
    create_waypoint(components, sum(pos, lin_comb(5.0f, w, 3.0f, h)));
    create_waypoint(components, sum(pos, mult(3.0f, w)));
    create_waypoint(components, sum(pos, lin_comb(3.0f, w, -3.0f, h)));
    create_waypoint(components, sum(pos, mult(-3.0f, w)));
    create_waypoint(components, sum(pos, lin_comb(-3.0f, w, -3.0f, h)));
    create_waypoint(components, sum(pos, mult(-5.0f, w)));
    create_waypoint(components, sum(pos, lin_comb(-5.0f, w, 3.0f, h)));

    int items = randi(1, 3);
    int locations[3] = {0, 1, 2};
    permute(locations, 3);
    for (int i = 0; i < items; i++) {
        switch (locations[i]) {
            case 0:
                create_item(components, sum(pos, lin_comb(-4.5f, w, 4.0f, h)), i);
                break;
            case 1:
                create_item(components, sum(pos, lin_comb(3.0f, w, 5.0f, h)), i);
                break;
            case 2:
                create_item(components, sum(pos, lin_comb(3.0f, w, -4.5f, h)), i);
                break;
        }
    }
}


void create_outhouse(ComponentData* components, sfVector2f pos) {
    float angle = randf(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 3.0f, 3.0f, angle, "board_tile");

    create_wall(components, sum(pos, mult(-1.25f, w)), angle + 0.5 * M_PI, 2.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, mult(1.25f, h)), angle, 3.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, mult(-1.25f, h)), angle, 3.0f, 0.5f, "wood_tile");

    create_wall(components, sum(pos, mult(-0.5f, w)), angle, 1.0f, 2.0f, "outhouse");

    create_waypoint(components, sum(pos, lin_comb(2.5f, w, 0.0f, h)));
    create_waypoint(components, sum(pos, lin_comb(2.5f, w, 2.5f, h)));
    create_waypoint(components, sum(pos, lin_comb(2.5f, w, -2.5f, h)));
    create_waypoint(components, sum(pos, lin_comb(-2.5f, w, 2.5f, h)));
    create_waypoint(components, sum(pos, lin_comb(-2.5f, w, -2.5f, h)));

    create_door(components, sum(pos, mult(1.25f, w)), angle + 0.5f * M_PI);

    create_item(components, pos, 0);
}


void create_ground(ComponentData* components, sfVector2f position, float width, float height, sfTexture* noise_texture) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0);
    ImageComponent_add(components, i, "grass_tile", width, height, LAYER_GROUND);

    i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0);
    ImageComponent* image = ImageComponent_add(components, i, "", 16.0, 16.0, LAYER_GROUND);
    image->scale = (sfVector2f) { 8.0, 8.0 };
    image->texture_changed = false;
    image->alpha = 1.0f;
    sfSprite_setTexture(image->sprite, noise_texture, false);
}


void create_water(ComponentData* components, sfVector2f position, float width, float height, sfTexture* noise_texture) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0f);
    ImageComponent_add(components, i, "water_tile", width, height, LAYER_GROUND);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_WALLS);

    i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0);
    ImageComponent* image = ImageComponent_add(components, i, "", 16.0, 16.0, LAYER_GROUND);
    image->scale = (sfVector2f) { 8.0f, 8.0f };
    image->texture_changed = false;
    image->alpha = 0.3f;
    sfSprite_setTexture(image->sprite, noise_texture, false);
}


void create_beach(ComponentData* components, sfVector2f position, float length, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "beach_tile", 16.0f, length, LAYER_GROUND);
}


void create_beach_corner(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "beach_corner", 0, 0, LAYER_ROADS);
}


void create_tree(ComponentData* components, ColliderGrid* grid, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());    
    float size = randf(1.0f, 1.5f);
    ColliderComponent_add_circle(components, i, 1.25f * size, GROUP_TREES);

    if (collides_with(components, grid, i)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "tree", 3.0, 3.0, LAYER_TREES)->scale = (sfVector2f) { size, size };
}


void create_rock(ComponentData* components, ColliderGrid* grid, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    float size = randf(0.75f, 2.0f);
    ColliderComponent_add_circle(components, i, 1.4 * size, GROUP_TREES);

    if (collides_with(components, grid, i)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "rock", 3.0, 3.0, LAYER_DECALS)->scale = (sfVector2f) { size, size };
}


void create_forest(ComponentData* components, ColliderGrid* grid, sfVector2f position, Permutation p, float forestation) {
    int n = 8;
    float m = n - 1;
    for (int i = 1; i < n; i++) {
        for (int j = 1; j < n; j++) {
            sfVector2f r = { (i - n / 2) * CHUNK_WIDTH / m, (j - n / 2) * CHUNK_HEIGHT / m };
            r = sum(sum(position, r), rand_vector());

            if ((1.0f - forestation) < perlin(0.03f * r.x, 0.03f * r.y, 0.0, p, -1)) {
                if (randf(0.0f, 1.0f) < 0.05f) {
                    create_rock(components, grid, r);
                } else {
                    create_tree(components, grid, r);
                }
            }
        }
    }
}


void create_uranium(ComponentData* components, ColliderGrid* grid, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    float r = randf(0.8, 1.2);
    ColliderComponent_add_circle(components, i, 0.5 * r, GROUP_TREES);

    if (collides_with(components, grid, i)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "uranium", r, r, LAYER_DECALS);
    LightComponent_add(components, i, randf(3.0, 5.0), 2.0 * M_PI, get_color(0.5, 1.0, 0.0, 1.0), 0.5, 1.0)->flicker = 0.25;
    SoundComponent_add(components, i, "");
    ParticleComponent* particle = ParticleComponent_add(components, i, 0.0, 2.0 * M_PI, 0.1, 0.05, 2.0, 5.0, 
                                                        get_color(0.5, 1.0, 0.0, 1.0), get_color(0.5, 1.0, 0.0, 1.0));
    particle->enabled = true;
    particle->loop = true;
}


void create_village(int chunks[LEVEL_WIDTH][LEVEL_HEIGHT]) {
    int k = randi(1, LEVEL_WIDTH - 2);
    int l = randi(1, LEVEL_WIDTH - 2);
    for (int i = k - 1; i <= k + 1; i++) {
        for (int j = l - 1; j <= l + 1; j++) {
            if (i == k && j == l) {
                chunks[i][j] = 1;
            } else {
                if (randi(0, 1) == 0) {
                    chunks[i][j] = 2;
                }
            }
        }
    }
}


void create_farm(int chunks[LEVEL_WIDTH][LEVEL_HEIGHT]) {
    bool barn = false;

    int k = randi(1, LEVEL_WIDTH - 2);
    int l = randi(1, LEVEL_WIDTH - 2);

    chunks[k][l] = 2;

    while (true) {
        int i = randi(k - 1, k + 1);
        int j = randi(l - 1, l + 1);

        if (chunks[i][j] == 0) {
            if (barn) {
                chunks[i][j] = 4;
                break;
            } else {
                chunks[i][j] = 3;
                barn = true;
            }
        }
    }
}


void create_level(ComponentData* components, ColliderGrid* grid, int seed) {
    srand(seed);

    Permutation perm;
    init_perlin(perm);

    int w = 512;
    int h = 512;

    sfTexture* noise_texture = sfTexture_create(w, h);

    sfUint8* pixels = malloc(sizeof(sfUint8) * w * h * 4);
    create_noise(pixels, w, h, zeros(), get_color(0.0f, 0.0f, 0.0f, 0.3f), 0.0f, perm);
    sfTexture_updateFromPixels(noise_texture, pixels, w, h, 0, 0);
    sfTexture_setRepeated(noise_texture, true);
    free(pixels);

    int chunks[LEVEL_WIDTH][LEVEL_HEIGHT] = { 0 };
    
    create_village(chunks);

    for (int i = 0; i < 3; i++) {
        create_farm(chunks);
    }

    for (int i = 0; i < LEVEL_WIDTH; i++) {
        for (int j = 0; j < LEVEL_HEIGHT; j++) {
            sfVector2f pos = { CHUNK_WIDTH * i + 0.5f * (1 - LEVEL_WIDTH) * CHUNK_WIDTH, CHUNK_HEIGHT * j + 0.5f * (1 - LEVEL_HEIGHT) * CHUNK_HEIGHT };
            if (i == 0 || j == 0 || i == LEVEL_WIDTH - 1 || j == LEVEL_WIDTH - 1) {
                create_water(components, pos, CHUNK_WIDTH, CHUNK_HEIGHT, noise_texture);
            } else {
                create_ground(components, pos, CHUNK_WIDTH, CHUNK_HEIGHT, noise_texture);
            }
        }
    }

    sfVector2f pos = { (1.5f - 0.5f * LEVEL_WIDTH) * CHUNK_WIDTH, (1.0f - 0.5f * LEVEL_HEIGHT) * CHUNK_HEIGHT };
    sfVector2f r = { 0.5f * CHUNK_WIDTH, 0.0f };
    float angle = 1.5f * M_PI;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2 * LEVEL_WIDTH - 5; j++) {
            create_beach(components, pos, 0.5f * CHUNK_WIDTH, angle);
            pos = sum(pos, r);
        }
        create_beach_corner(components, pos, angle + 0.5f * M_PI);
        angle += 0.5f * M_PI;
        r = rotate(r, 0.5f * M_PI);
        pos = sum(pos, r);
    }

    sfVector2f start = zeros();
    create_house(components, start);

    angle = rand_angle();
    int n = randi(5, 7);
    int buildings[7] = {1, 2, 2, 2, 3, 1, 3};
    permute(buildings, n);
    float rad = 0.5f * (LEVEL_WIDTH - 3) * CHUNK_WIDTH;

    sfVector2f prev_pos = zeros();
    int i = 0;
    while (i < n) {
        int m = randi(1, n - i);
        int k = randi(0, m - 1);
        for (int j = 0; j < m; j++) {
            pos = polar_to_cartesian(randf(0.75f, 1.0f) * rad, angle + i * 2.0f * M_PI / n);

            switch (buildings[i]) {
                case 1:
                    create_church(components, pos);
                    break;
                case 2:
                    create_house(components, pos);
                    break;
                case 3:
                    create_barn(components, pos);
                    break;
            }

            if (non_zero(prev_pos)) {
                create_road(components, pos, prev_pos);
            }
            if (j == k) {
                create_road(components, pos, start);
            }
            prev_pos = pos;
            i++;
        }
        prev_pos = zeros();
    }

    create_car(components, sum(start, mult(20.0f, rand_vector())));

    init_grid(components, grid);

    float f = 0.5f;
    for (int i = 1; i < LEVEL_WIDTH - 1; i++) {
        for (int j = 1; j < LEVEL_HEIGHT - 1; j++) {
            sfVector2f pos = { CHUNK_WIDTH * i + 0.5f * (1 - LEVEL_WIDTH) * CHUNK_WIDTH, CHUNK_HEIGHT * j + 0.5f * (1 - LEVEL_HEIGHT) * CHUNK_HEIGHT };

            create_forest(components, grid, pos, perm, f);

            for (int i = 0; i < 2; i++) {
                sfVector2f r = { randf(-0.5, 0.5) * CHUNK_WIDTH, randf(-0.5, 0.5) * CHUNK_HEIGHT };
                if (randf(0.0f, 1.0f) < 0.6f) {
                    create_zombie(components, grid, sum(pos, r));
                } else if (randf(0.0f, 1.0f) < 0.5f) {
                    create_farmer(components, grid, sum(pos, r));
                } else if (randf(0.0f, 1.0f) < 0.5f) {
                    create_priest(components, grid, sum(pos, r));
                } else {
                    create_big_boy(components, grid, sum(pos, r));
                }
            }

            // for (int i = 0; i < 2; i++) {
            //     sfVector2f r = { randf(-0.5, 0.5) * CHUNK_WIDTH, randf(-0.5, 0.5) * CHUNK_HEIGHT };
            //     create_uranium(components, grid, sum(pos, r));
            // }
        }
    }

    ColliderGrid_clear(grid);

    resize_roads(components);

    create_player(components, start, -1);
    // create_player(components, sum(start, (sfVector2f) { 0.0, -5.0 }), 0);
    // create_player(components, sum(start, (sfVector2f) { 4.0, -5.0 }), 1);
}


void test(ComponentData* components, ColliderGrid* grid) {
    Permutation perm;
    init_perlin(perm);

    int w = 512;
    int h = 512;

    sfTexture* noise_texture = sfTexture_create(w, h);

    sfVector2f position = zeros();
    create_ground(components, position, CHUNK_WIDTH, CHUNK_HEIGHT, noise_texture);

    init_grid(components, grid);

    sfVector2f start = zeros();
    create_car(components, start);
    // create_church(components, zeros());
    // create_outhouse(components, zeros());
    // create_church(components, zeros());
    // create_house(components, zeros());
    // create_bench(components, (sfVector2f) { 10.0f, 15.0f }, rand_angle());
    // create_priest(components, zeros());
    // create_car(components, zeros());
    // create_big_boy(components, (sfVector2f) { 5.0, 5.0 });
    create_zombie(components, grid, zeros());
    create_rope_gun(components, position);

    create_player(components, sum(start, (sfVector2f) { 2.0, -5.0 }), -1);
    // create_player(components, sum(start, (sfVector2f) { 2.0, -5.0 }), 0);
    // create_player(components, sum(start, (sfVector2f) { 2.0, -5.0 }), 1);

    create_axe(components, sum(start, (sfVector2f) { 5.0, -5.0 }));
    create_pistol(components, sum(start, (sfVector2f) { 7.0, -6.0 }));
    create_shotgun(components, sum(start, (sfVector2f) { 7.0, -5.0 }));
    create_assault_rifle(components, sum(start, (sfVector2f) { 6.0, -5.0 }));
    create_flashlight(components, sum(start, (sfVector2f) { 5.0, -6.0 }));
    create_ammo(components, sum(start, (sfVector2f) { 4.0, -8.0 }), AMMO_PISTOL);
    create_ammo(components, sum(start, (sfVector2f) { 5.0, -8.0 }), AMMO_RIFLE);
    create_ammo(components, sum(start, (sfVector2f) { 6.0, -8.0 }), AMMO_SHOTGUN);
}
