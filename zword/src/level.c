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
#include "building.h"


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
    int n = randi(5, 8);
    int buildings[8] = {5, 1, 2, 2, 3, 4, 2, 3};
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
                case 4:
                    create_mansion(components, pos);
                    break;
                case 5: 
                    create_school(components, pos);
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

    create_garage(components, sum(start, mult(25.0f, rand_vector())));

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

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            sfVector2f pos = { CHUNK_WIDTH * i + 0.5f * (1 - 5) * CHUNK_WIDTH, CHUNK_HEIGHT * j + 0.5f * (1 - 5) * CHUNK_HEIGHT };
            if (i == 0 || j == 0 || i == 5 - 1 || j == 5 - 1) {
                create_water(components, pos, CHUNK_WIDTH, CHUNK_HEIGHT, noise_texture);
            } else {
                create_ground(components, pos, CHUNK_WIDTH, CHUNK_HEIGHT, noise_texture);
            }
        }
    }

    init_grid(components, grid);

    sfVector2f start = zeros();
    // create_mansion(components, start);
    create_school(components, start);

    create_player(components, start, -1);
    create_assault_rifle(components, sum(start, rand_vector()));
    create_ammo(components, sum(start, rand_vector()), AMMO_RIFLE);
    create_ammo(components, sum(start, rand_vector()), AMMO_RIFLE);
    create_ammo(components, sum(start, rand_vector()), AMMO_RIFLE);
    // create_player(components, sum(start, (sfVector2f) { 0.0, -5.0 }), 0);
    // create_player(components, sum(start, (sfVector2f) { 4.0, -5.0 }), 1);
}
