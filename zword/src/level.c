#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>

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


#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32


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


void create_shed(ComponentData* component, sfVector2f pos) {
    float angle = randf(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(3.0, angle);
    sfVector2f h = perp(w);

    create_floor(component, pos, 6.0, 6.0, angle);

    create_wood_wall(component, sum(pos, w), 5.5, angle + 0.5 * M_PI);
    create_wood_wall(component, diff(pos, w), 5.5, angle + 0.5 * M_PI);
    create_wood_wall(component, diff(pos, h), 6.5, angle);

    create_wood_wall(component, sum(pos, sum(h, mult(2.0 / 3.0, w))), 2.5, angle + M_PI);
    create_wood_wall(component, sum(pos, diff(h, mult(2.0 / 3.0, w))), 2.5, angle + M_PI);

    create_enemy(component, sum(pos, polar_to_cartesian(2.0, randf(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, randf(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, randf(0.0, 2 * M_PI))));

    create_fire(component, sum(pos, mult(0.01, h)));

    create_waypoint(component, sum(pos, mult(0.75, h)));
    create_waypoint(component, sum(pos, mult(1.5, h)));
    create_waypoint(component, sum(pos, mult(1.5, sum(w, h))));
    create_waypoint(component, sum(pos, mult(1.5, diff(w, h))));
    create_waypoint(component, sum(pos, mult(-1.5, sum(w, h))));
    create_waypoint(component, sum(pos, mult(-1.5, diff(w, h))));

    create_roof(component, diff(pos, mult(0.665, h)), 8.0, 4.0, angle);
    create_roof(component, sum(pos, mult(0.665, h)), 8.0, 4.0, angle + M_PI);
}


void create_ground(ComponentData* components, sfVector2f position, float width, float height, Permutation p) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0);
    ImageComponent_add(components, i, "grass_tile", width, height, 0);

    return;

    i = create_entity(components);
    CoordinateComponent_add(components, i, position, 0.0);
    ImageComponent* image = ImageComponent_add(components, i, "", 4.0, 4.0, 0);
    image->texture_changed = false;
    image->scale = (sfVector2f) { CHUNK_WIDTH / 4.0, CHUNK_HEIGHT / 4.0 };

    int w = 512;
    int h = 512;
    sfUint8 pixels[512 * 512 * 4];
    int x = position.x / CHUNK_WIDTH + 5;
    int y = position.y / CHUNK_HEIGHT + 5;
    sfVector2f origin = { (w / PIXELS_PER_UNIT) * x, (h / PIXELS_PER_UNIT) * y };
    create_noise(pixels, w, h, origin, get_color(0.1, 0.1, 0.0, 0.75), p);

    sfTexture* texture = sfTexture_create(w, h);
    sfTexture_updateFromPixels(texture, pixels, w, h, 0, 0);
    sfTexture_setRepeated(texture, true);

    sfSprite_setTexture(image->sprite, texture, sfFalse);
    sfIntRect rect = { 0, 0, image->width * PIXELS_PER_UNIT, image->height * PIXELS_PER_UNIT };
    sfSprite_setTextureRect(image->sprite, rect);
}


void create_tree(ComponentData* components, sfVector2f position, float size) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    ImageComponent_add(components, i, "tree", 3.0, 3.0, 6)->scale = (sfVector2f) { size, size };
    ColliderComponent_add_circle(components, i, size, TREES);
}


void create_forest(ComponentData* components, sfVector2f position, Permutation p, float forestation) {
    int k = 0;
    for (int i = -5; i < 6; i++) {
        for (int j = -5; j < 6; j++) {
            sfVector2f r = { position.x + i * 3.0 + randf(-1.0, 1.0), position.y + j * 3.0 + randf(-1.0, 1.0) };

            if ((1.0 - forestation) < perlin(0.1 * r.x, 0.1 * r.y, 0.0, p)) {
                create_tree(components, r, randf(1.0, 1.5));
            } else {
                if (k % 4 == 0) {
                    create_waypoint(components, r);
                }
                k++;
            }
        }
    }
}


void create_chunk(ComponentData* components, sfVector2f position, Permutation p, float forestation) {
    create_ground(components, position, CHUNK_WIDTH, CHUNK_HEIGHT, p);

    if (forestation > 0.1) {
        create_forest(components, position, p, forestation);
    } else {
        create_house(components, position);

        // sfVector2f r = polar_to_cartesian(15.0, rand_angle());
        // create_shed(components, sum(position, r));
    }
}


void create_level(ComponentData* components, int seed) {
    srand(seed);

    Permutation p;
    init_perlin(p);

    for (int i = -3; i < 4; i++) {
        for (int j = -3; j < 4; j++) {
            float f = randf(0.0, 0.5);
            if (i == -3 || i == 3) {
                f = 1.0;
            } else if (j == -3 || j == 3) {
                f = 1.0;
            }

            sfVector2f position = { CHUNK_WIDTH * i, CHUNK_HEIGHT * j };
            create_chunk(components, position, p, f);
        }
    }

    create_player(components, (sfVector2f) { 5.0, 0.0 });

    create_car(components, 15.0, -5.0);
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
