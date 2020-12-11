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


void create_waypoint(ComponentData* components, sfVector2f pos) {
    int i = get_index(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    components->waypoint[i] = WaypointComponent_create();
}


void brick_wall(ComponentData* components, sfVector2f pos, float length, float angle) {
    int i = get_index(components);

    CoordinateComponent_add(components, i, pos, angle);
    components->collider[i] = ColliderComponent_create_rectangle(length, 0.75);
    ImageComponent_add(components, i, "brick_tile", length, 0.75, 2);
}


void wood_wall(ComponentData* component, sfVector2f pos, float length, float angle) {
    int i = get_index(component);

    CoordinateComponent_add(component, i, pos, angle);
    component->collider[i] = ColliderComponent_create_rectangle(length, 0.5);
    ImageComponent_add(component, i, "wood_tile", length, 0.5, 2);
}


void create_prop(ComponentData* component, sfVector2f pos) {
    int i = get_index(component);

    CoordinateComponent_add(component, i, pos, float_rand(0.0, 2 * M_PI));
    component->collider[i] = ColliderComponent_create_rectangle(1.0, 1.0);
    component->physics[i] = PhysicsComponent_create(1.0, 0.5, 0.5, 2.0, 4.0);
}


void create_fire(ComponentData* component, sfVector2f pos) {
    int i = get_index(component);

    CoordinateComponent_add(component, i, pos, 0.0);
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    sfColor yellow = get_color(1.0, 1.0, 0.0, 1.0);
    component->light[i] = LightComponent_create(10.0, 2.0 * M_PI, 201, orange, 0.5, 10.0);
    component->particle[i] = ParticleComponent_create(0.5 * M_PI, 1.0, 0.8, 0.2, 1.0, 5.0, orange, yellow);
    component->particle[i]->loop = true;
    component->particle[i]->enabled = true;
    component->collider[i] = ColliderComponent_create_circle(0.35);
    ImageComponent_add(component, i, "fire", 1.0, 1.0, 5);
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
}


void create_flashlight(ComponentData* component, float x, float y) {
    int i = get_index(component);

    sfVector2f pos = { x, y };

    CoordinateComponent_add(component, i, pos, float_rand(0.0, 2 * M_PI));
    //component->collider[i] = ColliderComponent_create_rectangle(1.0, 0.25);
    component->physics[i] = PhysicsComponent_create(0.5, 0.0, 0.5, 10.0, 2.5);
    component->item[i] = ItemComponent_create(0);
    component->light[i] = LightComponent_create(7.0, 1.0, 51, sfColor_fromRGB(255, 255, 200), 0.75, 10.0);
    component->light[i]->enabled = false;
    ImageComponent_add(component, i, "flashlight", 1.0, 1.0, 3);
}


void create_level(ComponentData* component) {
    create_shed(component, -20.0, 0.0);
    create_shed(component, 20.0, 0.0);
    create_house(component, 10.0, 10.0);

    create_player(component, (sfVector2f) { 0.0, 0.0 });

    create_car(component, 0.0, -5.0);
    create_weapon(component, 1.0, 5.0);
    create_weapon(component, 1.0, 4.0);

    create_flashlight(component, 0.0, 2.0);
    create_flashlight(component, 0.0, 3.0);
    create_flashlight(component, 0.0, 4.0);
    create_flashlight(component, 0.0, 5.0);
    create_flashlight(component, 0.0, 6.0);

    create_lasersight(component, -2.0, 0.0);
}
