#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>

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


void create_waypoint(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    WaypointComponent_add(components, i);
}


void create_brick_wall(ComponentData* components, sfVector2f pos, float length, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, length, 0.75, WALLS);
    ImageComponent_add(components, i, "brick_tile", length, 0.75, 2);
    ParticleComponent_add_dirt(components, i);
}


void create_wood_wall(ComponentData* components, sfVector2f pos, float length, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, length, 0.5, WALLS);
    ImageComponent_add(components, i, "wood_tile", length, 0.5, 2);
    ParticleComponent_add_dirt(components, i);
}


void create_wall(ComponentData* components, sfVector2f pos, float angle, float width, float height, Filename filename) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, width, height, WALLS);
    ImageComponent_add(components, i, filename, width, height, 2);
    ParticleComponent_add_dirt(components, i);
}


void create_fire(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(components, i, 5.0, 2.0 * M_PI, orange, 0.8, 10.0)->flicker = 0.2;
    ParticleComponent* particle = ParticleComponent_add(components, i, 0.5 * M_PI, 1.0, 0.6, 0.2, 1.0, 5.0, orange, orange);
    particle->loop = true;
    particle->enabled = true;
    ColliderComponent_add_circle(components, i, 0.35, WALLS);
    ImageComponent_add(components, i, "fire", 1.0, 1.0, 5);
}


void create_light(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, rand_angle());
    ParticleComponent_add_fire(components, i, 0.25f);
    ImageComponent_add(components, i, "candle", 1.0f, 1.0f, 3);
}


void create_floor(ComponentData* components, sfVector2f pos, float width, float height, float angle, Filename filename) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, angle);
    ImageComponent_add(components, i, filename, width, height, 1);
    ColliderComponent_add_rectangle(components, i, width, height, GROUP_FLOORS);
}


void create_roof(ComponentData* component, sfVector2f pos, float width, float height, float angle) {
    int i = create_entity(component);

    CoordinateComponent_add(component, i, pos, angle);
    ImageComponent_add(component, i, "roof_tile", width, height, 6);
}


void create_item(ComponentData* components, sfVector2f position) {
    switch (rand() % 10) {
        case 0:
            create_pistol(components, position);
            break;
        case 1:
            create_assault_rifle(components, position);
            break;
        case 2:
            create_shotgun(components, position);
            break;
        case 3:
            create_gas(components, position);
            break;
        case 4:
            create_flashlight(components, position);
            break;
        default:
            create_ammo(components, position, rand() % 3 + 1);
            break;
    }
}


void create_bench(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "bench", 1.0f, 3.0f, 3);
    ColliderComponent_add_rectangle(components, i, 0.8f, 2.8f, WALLS);
}


void create_hay_bale(ComponentData* components, sfVector2f position, float angle) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, angle);
    ImageComponent_add(components, i, "hay_bale", 3.0f, 2.0f, 3);
    ColliderComponent_add_rectangle(components, i, 2.8f, 1.5f, WALLS);
}


void create_decal(ComponentData* components, sfVector2f pos, float width, float height, Filename filename) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, rand_angle());
    ImageComponent* image = ImageComponent_add(components, i, filename, width, height, 2);
}


void create_church(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = polar_to_cartesian(1.0f, angle + 0.5f * M_PI);

    create_floor(components, pos, 30.0f, 10.0f, angle, "tiles_tile");
    create_floor(components, sum(pos, mult(5.0f, w)), 10.0f, 20.0f, angle, "tiles_tile");

    // Back wall
    create_wall(components, sum(pos, mult(14.5f, w)), angle + 0.5f * M_PI, 8.0f, 1.0f, "stone_tile");

    // Altar
    create_wall(components, sum(pos, mult(12.5f, w)), angle, 3.0f, 8.0f, "altar_tile");
    create_light(components, sum(pos, sum(mult(12.5f, w), mult(-3.0f, h))));
    create_light(components, sum(pos, sum(mult(12.5f, w), mult(3.0f, h))));
    create_decal(components, sum(pos, mult(12.5f, w)), 2.0f, 2.0f, "blood_large");

    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 7; i++) {
            create_bench(components, sum(pos, sum(mult((i - 5.75f) * 2.0f, w), mult(2.5f, h))), angle);
        }

        // Entrance
        create_wall(components, sum(pos, sum(mult(-14.5f, w), mult(2.5f, h))), angle + 0.5f * M_PI, 3.0f, 1.0f, "stone_tile");
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

        create_item(components, sum(pos, sum(mult(5.0f, w), mult(6.5f, h))));

        h = mult(-1.0f, h);
    }

    create_waypoint(components, sum(pos, mult(-16.0f, w)));
    create_waypoint(components, sum(pos, mult(-12.0f, w)));
    create_waypoint(components, sum(pos, mult(2.0f, w)));
    create_waypoint(components, sum(pos, mult(8.0f, w)));
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

        create_item(components, sum(pos, lin_comb(4.0f, w, 4.0f, h)));

        // Outside
        create_waypoint(components, sum(pos, lin_comb(-13.5f, w, 8.0f, h)));
        create_waypoint(components, sum(pos, lin_comb(0.0f, w, 8.0f, h)));
        create_waypoint(components, sum(pos, lin_comb(13.5f, w, 8.0f, h)));

        h = mult(-1.0f, h);
    }

    create_waypoint(components, sum(pos, mult(-11.0f, w)));
    create_waypoint(components, sum(pos, mult(-13.5f, w)));
    create_waypoint(components, sum(pos, mult(13.5f, w)));

    create_item(components, sum(pos, mult(4.0f, w)));
}


void create_house(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(5.0, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 10.0, 10.0, angle, "board_tile");

    create_brick_wall(components, sum(pos, sum(w, mult(0.55, h))), 3.75, angle + 0.5 * M_PI);
    create_brick_wall(components, sum(pos, sum(w, mult(-0.55, h))), 3.75, angle + 0.5 * M_PI);

    create_brick_wall(components, diff(pos, w), 9.25, angle + 0.5 * M_PI);
    create_brick_wall(components, diff(pos, h), 10.75, angle);
    create_brick_wall(components, sum(pos, h), 10.75, angle);

    create_wood_wall(components, sum(pos, mult(-0.28, h)), 6.5, angle + 0.5 * M_PI);
    create_wood_wall(components, sum(pos, mult(0.78, h)), 1.5, angle + 0.5 * M_PI);

    create_wood_wall(components, sum(pos, mult(-0.78, w)), 1.5, angle);
    create_wood_wall(components, sum(pos, mult(-0.2, w)), 1.5, angle);

    create_waypoint(components, sum(pos, mult(0.75, w)));
    create_waypoint(components, sum(pos, mult(1.5, w)));

    create_waypoint(components, sum(pos, mult(1.5, sum(w, h))));
    create_waypoint(components, sum(pos, mult(1.5, diff(w, h))));
    create_waypoint(components, sum(pos, mult(-1.5, sum(w, h))));
    create_waypoint(components, sum(pos, mult(-1.5, diff(w, h))));

    create_waypoint(components, sum(pos, mult(0.5, sum(w, h))));
    create_waypoint(components, diff(pos, mult(0.5, sum(w, h))));
    create_waypoint(components, diff(pos, mult(0.5, diff(w, h))));

    create_light(components, sum(pos, mult(0.8f, diff(w, h))));

    create_item(components, sum(pos, mult(0.5f, diff(w, h))));
    create_item(components, diff(pos, mult(0.5f, diff(w, h))));
    create_item(components, diff(pos, mult(0.5f, sum(w, h))));
}


void create_toilet(ComponentData* components, sfVector2f pos) {
    float angle = randf(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 3.0f, 3.0f, angle, "board_tile");

    create_wall(components, sum(pos, mult(-1.25f, w)), angle + 0.5 * M_PI, 2.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, mult(1.25f, h)), angle, 3.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, mult(-1.25f, h)), angle, 3.0f, 0.5f, "wood_tile");

    create_wall(components, sum(pos, mult(-0.5f, w)), angle, 1.0f, 2.0f, "toilet");

    create_waypoint(components, sum(pos, lin_comb(2.5f, w, 0.0f, h)));
    create_waypoint(components, sum(pos, lin_comb(2.5f, w, 2.5f, h)));
    create_waypoint(components, sum(pos, lin_comb(2.5f, w, -2.5f, h)));
    create_waypoint(components, sum(pos, lin_comb(-2.5f, w, 2.5f, h)));
    create_waypoint(components, sum(pos, lin_comb(-2.5f, w, -2.5f, h)));
}


void create_ground(ComponentData* components, sfVector2f position, float width, float height, sfTexture* noise_texture) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0);
    ImageComponent_add(components, i, "grass_tile", width, height, 0);

    i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0);
    ImageComponent* image = ImageComponent_add(components, i, "", 16.0, 16.0, 0);
    image->scale = (sfVector2f) { 8.0, 8.0 };
    image->texture_changed = false;
    sfSprite_setTexture(image->sprite, noise_texture, false);
}


void create_tree(ComponentData* components, ColliderGrid* grid, sfVector2f position, float size) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_circle(components, i, size, TREES);

    if (collides_with(components, grid, i, ROADS) || collides_with(components, grid, i, WALLS) || collides_with(components, grid, i, GROUP_FLOORS)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "tree", 3.0, 3.0, 6)->scale = (sfVector2f) { size, size };
}


void create_rock(ComponentData* components, ColliderGrid* grid, sfVector2f position, float size) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_circle(components, i, 1.4 * size, TREES);

    if (collides_with(components, grid, i, ROADS) || collides_with(components, grid, i, WALLS) || collides_with(components, grid, i, GROUP_FLOORS)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "rock", 3.0, 3.0, 2)->scale = (sfVector2f) { size, size };
}


void create_uranium(ComponentData* components, ColliderGrid* grid, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    float r = randf(0.8, 1.2);
    ColliderComponent_add_circle(components, i, 0.5 * r, TREES);

    if (collides_with(components, grid, i, ROADS) || collides_with(components, grid, i, WALLS) || collides_with(components, grid, i, GROUP_FLOORS)) {
        destroy_entity(components, i);
        return;
    }

    ImageComponent_add(components, i, "uranium", r, r, 2);
    LightComponent_add(components, i, randf(3.0, 5.0), 2.0 * M_PI, get_color(0.5, 1.0, 0.0, 1.0), 0.5, 1.0)->flicker = 0.25;
    SoundComponent_add(components, i, "");
    ParticleComponent* particle = ParticleComponent_add(components, i, 0.0, 2.0 * M_PI, 0.1, 0.05, 2.0, 5.0, 
                                                        get_color(0.5, 1.0, 0.0, 1.0), get_color(0.5, 1.0, 0.0, 1.0));
    particle->enabled = true;
    particle->loop = true;
}


void create_forest(ComponentData* components, ColliderGrid* grid, sfVector2f position, Permutation p, float forestation) {
    for (int i = -5; i < 6; i++) {
        for (int j = -5; j < 6; j++) {
            sfVector2f r = { position.x + i * 3.0 + randf(-1.0, 1.0), position.y + j * 3.0 + randf(-1.0, 1.0) };

            if ((1.0 - forestation) < perlin(0.03 * r.x, 0.03 * r.y, 0.0, p, -1)) {
                if (randf(0.0, 1.0) < 0.05) {
                    create_rock(components, grid, r, randf(0.75, 2.0));
                } else {
                    create_tree(components, grid, r, randf(1.0, 1.5));
                }
            }
        }
    }
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
    create_noise(pixels, w, h, zeros(), get_color(0.0, 0.0, 0.0, 0.3), perm);
    sfTexture_updateFromPixels(noise_texture, pixels, w, h, 0, 0);
    sfTexture_setRepeated(noise_texture, true);
    free(pixels);

    int chunks[LEVEL_WIDTH][LEVEL_HEIGHT] = { 0 };
    
    create_village(chunks);

    for (int i = 0; i < 3; i++) {
        create_farm(chunks);
    }

    sfVector2f start = zeros();

    for (int i = 0; i < LEVEL_WIDTH; i++) {
        for (int j = 0; j < LEVEL_HEIGHT; j++) {
            sfVector2f pos = { CHUNK_WIDTH * i - 0.5f * LEVEL_WIDTH * CHUNK_WIDTH, CHUNK_HEIGHT * j - 0.5f * LEVEL_HEIGHT * CHUNK_HEIGHT };
            create_ground(components, pos, CHUNK_WIDTH, CHUNK_HEIGHT, noise_texture);

            switch (chunks[i][j]) {
                case 1:
                    create_church(components, pos);
                    create_road(components, start, pos, perm);
                    break;
                case 2:
                    create_house(components, pos);
                    break;
                case 3:
                    create_barn(components, pos);
                    break;
                case 4:
                    create_toilet(components, pos);
                    break;
                
                default:
                    break;
            }
        }
    }

    init_grid(components, grid);

    float f = 0.5f;
    for (int i = 0; i < LEVEL_WIDTH; i++) {
        for (int j = 0; j < LEVEL_HEIGHT; j++) {
            sfVector2f pos = { CHUNK_WIDTH * i - 0.5f * LEVEL_WIDTH * CHUNK_WIDTH, CHUNK_HEIGHT * j - 0.5f * LEVEL_HEIGHT * CHUNK_HEIGHT };

            switch (chunks[i][j]) {
                case 1:
                    f = 0.1f;
                    break;
                case 2:
                    f = 0.1f;
                    break;
                case 3:
                    f = 0.2f;
                    break;
                case 4:
                    f = 0.3f;
                    break;
                default:
                    f = 0.5f;
                    break;
            }

            create_forest(components, grid, pos, perm, f);

            for (int i = 0; i < 4; i++) {
                sfVector2f r = { randf(-0.5, 0.5) * CHUNK_WIDTH, randf(-0.5, 0.5) * CHUNK_HEIGHT };
                // create_enemy(components, sum(pos, r));
            }
        }
    }

    // for (int i = -width; i <= width; i++) {
    //     for (int j = -height; j <= height; j++) {
    //         if (i == 0 && j == 0) continue;
            
    //         sfVector2f position = { CHUNK_WIDTH * i, CHUNK_HEIGHT * j };

    //         float f = randf(0.0, 0.75);
    //         // create_forest(components, grid, position, perm, f);

    //         // for (int i = 0; i < 4; i++) {
    //         //     sfVector2f r = { randf(-0.5, 0.5) * CHUNK_WIDTH, randf(-0.5, 0.5) * CHUNK_HEIGHT };
    //         //     create_enemy(components, sum(position, r));
    //         //     // create_big_boy(components, sum(position, r));
    //         // }

    //         // for (int i = 0; i < 2; i++) {
    //         //     sfVector2f r = { randf(-0.5, 0.5) * CHUNK_WIDTH, randf(-0.5, 0.5) * CHUNK_HEIGHT };
    //         //     create_uranium(components, grid, sum(position, r));
    //         // }
    //     }
    // }

    resize_roads(components);

    create_player(components, sum(start, (sfVector2f) { 2.0, -5.0 }), -1);
    // create_player(components, sum(start, (sfVector2f) { 0.0, -5.0 }), 0);
    // create_player(components, sum(start, (sfVector2f) { 4.0, -5.0 }), 1);
    create_car(components, start);
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
    // create_car(components, start);
    // create_church(components, zeros());
    create_toilet(components, zeros());

    // create_big_boy(components, (sfVector2f) { 5.0, 5.0 });

    create_player(components, sum(start, (sfVector2f) { 2.0, -5.0 }), -1);

    for (int i = 0; i < 2; i++) {
        create_axe(components, sum(start, (sfVector2f) { 5.0, -5.0 }));
        create_pistol(components, sum(start, (sfVector2f) { 7.0, -6.0 }));
        create_shotgun(components, sum(start, (sfVector2f) { 7.0, -5.0 }));
        create_assault_rifle(components, sum(start, (sfVector2f) { 6.0, -5.0 }));
        create_flashlight(components, sum(start, (sfVector2f) { 5.0, -6.0 }));
        create_ammo(components, sum(start, (sfVector2f) { 4.0, -8.0 }), AMMO_PISTOL);
        create_ammo(components, sum(start, (sfVector2f) { 5.0, -8.0 }), AMMO_RIFLE);
        create_ammo(components, sum(start, (sfVector2f) { 6.0, -8.0 }), AMMO_SHOTGUN);
    }
}
