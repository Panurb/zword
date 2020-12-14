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


void create_waypoint(ComponentData* components, sfVector2f pos) {
    int i = get_index(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    components->waypoint[i] = WaypointComponent_create();
}


void brick_wall(ComponentData* components, sfVector2f pos, float length, float angle) {
    int i = get_index(components);

    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, length, 0.75, WALLS);
    ImageComponent_add(components, i, "brick_tile", length, 0.75, 2);
}


void wood_wall(ComponentData* components, sfVector2f pos, float length, float angle) {
    int i = get_index(components);

    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_rectangle(components, i, length, 0.5, WALLS);
    ImageComponent_add(components, i, "wood_tile", length, 0.5, 2);
}


void create_prop(ComponentData* components, sfVector2f pos) {
    int i = get_index(components);

    CoordinateComponent_add(components, i, pos, float_rand(0.0, 2 * M_PI));
    ColliderComponent_add_rectangle(components, i, 1.0, 1.0, WALLS);
    PhysicsComponent_add(components, i, 1.0, 0.5, 0.5, 2.0, 4.0);
}


void create_fire(ComponentData* components, sfVector2f pos) {
    int i = get_index(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    sfColor yellow = get_color(1.0, 1.0, 0.0, 1.0);
    components->light[i] = LightComponent_create(5.0, 2.0 * M_PI, 201, orange, 0.8, 10.0);
    components->light[i]->flicker = 0.2;
    components->particle[i] = ParticleComponent_create(0.5 * M_PI, 1.0, 0.8, 0.2, 1.0, 5.0, orange, yellow);
    components->particle[i]->loop = true;
    components->particle[i]->enabled = true;
    ColliderComponent_add_circle(components, i, 0.35, WALLS);
    ImageComponent_add(components, i, "fire", 1.0, 1.0, 5);
}


void create_light(ComponentData* component, sfVector2f pos) {
    int i = get_index(component);

    CoordinateComponent_add(component, i, pos, 0.0);
    component->light[i] = LightComponent_create(10.0, 2.0 * M_PI, 201, sfColor_fromRGB(255, 255, 150), 0.4, 10.0);
}


void create_floor(ComponentData* component, sfVector2f pos, float width, float height, float angle) {
    int i = get_index(component);

    CoordinateComponent_add(component, i, pos, angle);
    ImageComponent_add(component, i, "board_tile", width, height, 1);
}


void create_roof(ComponentData* component, sfVector2f pos, float width, float height, float angle) {
    int i = get_index(component);

    CoordinateComponent_add(component, i, pos, angle);
    ImageComponent_add(component, i, "roof_tile", width, height, 6);
}


void create_house(ComponentData* component, float x, float y) {
    float angle = float_rand(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(5.0, angle);
    sfVector2f h = perp(w);

    sfVector2f pos = { x, y };

    create_floor(component, pos, 10.0, 10.0, angle);

    brick_wall(component, sum(pos, sum(w, mult(0.55, h))), 3.75, angle + 0.5 * M_PI);
    brick_wall(component, sum(pos, sum(w, mult(-0.55, h))), 3.75, angle + 0.5 * M_PI);

    brick_wall(component, diff(pos, w), 9.25, angle + 0.5 * M_PI);
    brick_wall(component, diff(pos, h), 10.75, angle);
    brick_wall(component, sum(pos, h), 10.75, angle);

    wood_wall(component, sum(pos, mult(-0.28, h)), 6.5, angle + 0.5 * M_PI);
    wood_wall(component, sum(pos, mult(0.78, h)), 1.5, angle + 0.5 * M_PI);

    wood_wall(component, sum(pos, mult(-0.78, w)), 1.5, angle);
    wood_wall(component, sum(pos, mult(-0.2, w)), 1.5, angle);

    create_light(component, diff(pos, mult(0.51, sum(w, h))));
    create_light(component, sum(pos, mult(0.51, diff(w, h))));
    create_light(component, diff(pos, mult(0.51, diff(w, h))));

    create_waypoint(component, sum(pos, mult(0.75, w)));
    create_waypoint(component, sum(pos, mult(1.5, w)));

    create_waypoint(component, sum(pos, mult(1.5, sum(w, h))));
    create_waypoint(component, sum(pos, mult(1.5, diff(w, h))));
    create_waypoint(component, sum(pos, mult(-1.5, sum(w, h))));
    create_waypoint(component, sum(pos, mult(-1.5, diff(w, h))));

    create_waypoint(component, sum(pos, mult(0.5, sum(w, h))));
    create_waypoint(component, diff(pos, mult(0.5, sum(w, h))));
    create_waypoint(component, diff(pos, mult(0.5, diff(w, h))));
}


void create_shed(ComponentData* component, float x, float y) {
    float angle = float_rand(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(3.0, angle);
    sfVector2f h = perp(w);

    sfVector2f pos = { x, y };

    create_floor(component, pos, 6.0, 6.0, angle);

    wood_wall(component, sum(pos, w), 5.5, angle + 0.5 * M_PI);
    wood_wall(component, diff(pos, w), 5.5, angle + 0.5 * M_PI);
    wood_wall(component, diff(pos, h), 6.5, angle);

    wood_wall(component, sum(pos, sum(h, mult(2.0 / 3.0, w))), 2.5, angle + M_PI);
    wood_wall(component, sum(pos, diff(h, mult(2.0 / 3.0, w))), 2.5, angle + M_PI);

    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));

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


void create_ground(ComponentData* components, float width, float height) {
    int i = get_index(components);
    CoordinateComponent_add(components, i, zeros(), 0.0);
    ImageComponent_add(components, i, "grass_tile", width, height, 0);

    i = get_index(components);
    CoordinateComponent_add(components, i, zeros(), 0.0);
    ImageComponent* image = ImageComponent_add(components, i, "", 4.0, 4.0, 0);
    image->texture_changed = false;
    image->scale = mult(16.0, ones());

    int perm[512];
    init_perlin(perm);

    int w = 512;
    int h = 512;
    sfUint8 pixels[512 * 512 * 4];
    create_noise(pixels, w, h, get_color(0.1, 0.1, 0.0, 0.75), perm);

    sfTexture* texture = sfTexture_create(w, h);
    sfTexture_updateFromPixels(texture, pixels, w, h, 0, 0);
    sfTexture_setRepeated(texture, true);

    sfSprite_setTexture(image->sprite, texture, sfFalse);
    sfIntRect rect = {0, 0, image->width * PIXELS_PER_UNIT, image->height * PIXELS_PER_UNIT };
    sfSprite_setTextureRect(image->sprite, rect);
}


void create_level(ComponentData* components, float width, float height) {
    create_ground(components, width, height);

    create_shed(components, -20.0, 0.0);
    create_shed(components, 20.0, 0.0);
    create_house(components, 10.0, 10.0);

    create_player(components, (sfVector2f) { 0.0, 0.0 });

    create_car(components, 0.0, -5.0);
    create_weapon(components, 1.0, 5.0);
    create_weapon(components, 1.0, 4.0);

    create_flashlight(components, 0.0, 2.0);
}


void test(ComponentData* components) {
    for (int i = 0; i < 20; i ++) {
        for (int j = 0; j < 20; j++) {
            int k = get_index(components);

            sfVector2f pos = { i, j };

            CoordinateComponent_add(components, k, pos, 0.0);
            components->waypoint[k] = WaypointComponent_create();
        }
    }
}
