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

    CoordinateComponent_add(components, i, pos, 0.0);
    LightComponent_add(components, i, 10.0, 2.0 * M_PI, get_color(1.0, 1.0, 0.6, 1.0), 0.4, 10.0);
}


void create_floor(ComponentData* component, sfVector2f pos, float width, float height, float angle) {
    int i = create_entity(component);

    CoordinateComponent_add(component, i, pos, angle);
    ImageComponent_add(component, i, "board_tile", width, height, 1);
}


void create_roof(ComponentData* component, sfVector2f pos, float width, float height, float angle) {
    int i = create_entity(component);

    CoordinateComponent_add(component, i, pos, angle);
    ImageComponent_add(component, i, "roof_tile", width, height, 6);
}


void create_house(ComponentData* components, sfVector2f pos) {
    float angle = randf(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(5.0, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 10.0, 10.0, angle);

    create_brick_wall(components, sum(pos, sum(w, mult(0.55, h))), 3.75, angle + 0.5 * M_PI);
    create_brick_wall(components, sum(pos, sum(w, mult(-0.55, h))), 3.75, angle + 0.5 * M_PI);

    create_brick_wall(components, diff(pos, w), 9.25, angle + 0.5 * M_PI);
    create_brick_wall(components, diff(pos, h), 10.75, angle);
    create_brick_wall(components, sum(pos, h), 10.75, angle);

    create_wood_wall(components, sum(pos, mult(-0.28, h)), 6.5, angle + 0.5 * M_PI);
    create_wood_wall(components, sum(pos, mult(0.78, h)), 1.5, angle + 0.5 * M_PI);

    create_wood_wall(components, sum(pos, mult(-0.78, w)), 1.5, angle);
    create_wood_wall(components, sum(pos, mult(-0.2, w)), 1.5, angle);

    create_light(components, diff(pos, mult(0.51, sum(w, h))));
    create_light(components, sum(pos, mult(0.51, diff(w, h))));
    create_light(components, diff(pos, mult(0.51, diff(w, h))));

    create_pistol(components, sum(pos, mult(0.51, diff(w, h))));
    create_flashlight(components, diff(pos, mult(0.51, diff(w, h))));

    create_waypoint(components, sum(pos, mult(0.75, w)));
    create_waypoint(components, sum(pos, mult(1.5, w)));

    create_waypoint(components, sum(pos, mult(1.5, sum(w, h))));
    create_waypoint(components, sum(pos, mult(1.5, diff(w, h))));
    create_waypoint(components, sum(pos, mult(-1.5, sum(w, h))));
    create_waypoint(components, sum(pos, mult(-1.5, diff(w, h))));

    create_waypoint(components, sum(pos, mult(0.5, sum(w, h))));
    create_waypoint(components, diff(pos, mult(0.5, sum(w, h))));
    create_waypoint(components, diff(pos, mult(0.5, diff(w, h))));
}


void create_shed(ComponentData* components, sfVector2f pos) {
    float angle = randf(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(3.0, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 6.0, 6.0, angle);

    create_wood_wall(components, sum(pos, w), 5.5, angle + 0.5 * M_PI);
    create_wood_wall(components, diff(pos, w), 5.5, angle + 0.5 * M_PI);
    create_wood_wall(components, diff(pos, h), 6.5, angle);

    create_wood_wall(components, sum(pos, sum(h, mult(2.0 / 3.0, w))), 2.5, angle + M_PI);
    create_wood_wall(components, sum(pos, diff(h, mult(2.0 / 3.0, w))), 2.5, angle + M_PI);

    create_enemy(components, sum(pos, polar_to_cartesian(2.0, randf(0.0, 2 * M_PI))));
    create_enemy(components, sum(pos, polar_to_cartesian(2.0, randf(0.0, 2 * M_PI))));
    create_enemy(components, sum(pos, polar_to_cartesian(2.0, randf(0.0, 2 * M_PI))));

    create_fire(components, sum(pos, mult(0.01, h)));

    create_waypoint(components, sum(pos, mult(0.75, h)));
    create_waypoint(components, sum(pos, mult(1.5, h)));
    create_waypoint(components, sum(pos, mult(1.5, sum(w, h))));
    create_waypoint(components, sum(pos, mult(1.5, diff(w, h))));
    create_waypoint(components, sum(pos, mult(-1.5, sum(w, h))));
    create_waypoint(components, sum(pos, mult(-1.5, diff(w, h))));

    create_gas(components, pos);

    // create_roof(component, diff(pos, mult(0.665, h)), 8.0, 4.0, angle);
    // create_roof(component, sum(pos, mult(0.665, h)), 8.0, 4.0, angle + M_PI);
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
    ImageComponent_add(components, i, "tree", 3.0, 3.0, 6)->scale = (sfVector2f) { size, size };
    ColliderComponent* col = ColliderComponent_add_circle(components, i, 2.0 * size, TREES);

    if (collides_with(components, grid, i, ROADS) || collides_with(components, grid, i, WALLS)) {
        clear_grid(components, grid, i);
        destroy_entity(components, i);
    } else {
        col->radius = size;
        col->width = 2.0 * col->radius;
        col->height = 2.0 * col->radius;
    }
}


void create_rock(ComponentData* components, ColliderGrid* grid, sfVector2f position, float size) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    ImageComponent_add(components, i, "rock", 3.0, 3.0, 2)->scale = (sfVector2f) { size, size };
    ColliderComponent* col = ColliderComponent_add_circle(components, i, 2.8 * size, TREES);

    if (collides_with(components, grid, i, ROADS) || collides_with(components, grid, i, WALLS)) {
        clear_grid(components, grid, i);
        destroy_entity(components, i);
    } else {
        col->radius = 1.4 * size;
        col->width = 2.0 * col->radius;
        col->height = 2.0 * col->radius;
    }
}


void create_uranium(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    float r = randf(0.8, 1.2);
    ImageComponent_add(components, i, "uranium", r, r, 2);
    ColliderComponent_add_circle(components, i, 0.5 * r, TREES);
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

            if ((1.0 - forestation) < perlin(0.1 * r.x, 0.1 * r.y, 0.0, p, -1)) {
                if (randf(0.0, 1.0) < 0.05) {
                    create_rock(components, grid, r, randf(0.75, 2.0));
                } else {
                    create_tree(components, grid, r, randf(1.0, 1.5));
                }
            }
        }
    }
}


void create_chunk(ComponentData* components, ColliderGrid* grid, sfVector2f position, Permutation p, float forestation, sfTexture* noise_texture) {
    create_ground(components, position, CHUNK_WIDTH, CHUNK_HEIGHT, noise_texture);
    create_forest(components, grid, position, p, forestation);

    for (int i = 0; i < 4; i++) {
        sfVector2f r = { randf(-0.5, 0.5) * CHUNK_WIDTH, randf(-0.5, 0.5) * CHUNK_HEIGHT };
        create_enemy(components, sum(position, r));
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

    sfVector2f end = { randf(-4.5, 4.5) * CHUNK_WIDTH, randf(-4.5, 4.5) * CHUNK_HEIGHT };
    create_house(components, end);

    sfVector2f start = { (randf(0.0, 9.0) - 4.5) * CHUNK_WIDTH, -4.0 * CHUNK_HEIGHT };
    create_road(components, start, end, perm);

    init_grid(components, grid);

    float f = randf(0.0, 1.0);
    for (int i = -4; i < 5; i++) {
        for (int j = -4; j < 5; j++) {
            f = randf(0.0, 0.75);

            sfVector2f position = { CHUNK_WIDTH * i, CHUNK_HEIGHT * j };
            create_chunk(components, grid, position, perm, f, noise_texture);
        }
    }

    create_player(components, sum(start, (sfVector2f) { 0.0, -5.0 }));

    create_car(components, start);

    create_pistol(components, sum(start, (sfVector2f) { 5.0, -5.0 }));
}


void test(ComponentData* components) {
    for (int i = 0; i < 20; i ++) {
        for (int j = 0; j < 20; j++) {
            int k = create_entity(components);

            sfVector2f pos = { i, j };

            CoordinateComponent_add(components, k, pos, 0.0);
            WaypointComponent_add(components, k);
        }
    }
}
