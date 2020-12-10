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


void create_waypoint(ComponentData* component, sfVector2f pos) {
    int i = get_index(component);

    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->waypoint[i] = WaypointComponent_create();
    component->collider[i] = ColliderComponent_create_circle(0.5);
}


void create_wall(ComponentData* component, sfVector2f pos, float width, float height, float angle) {
    int i = get_index(component);

    component->coordinate[i] = CoordinateComponent_create(pos, angle);
    component->collider[i] = ColliderComponent_create_rectangle(width, height);
    component->image[i] = ImageComponent_create("brick_tile", width, height, 2);
}


void create_prop(ComponentData* component, sfVector2f pos) {
    int i = get_index(component);

    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->collider[i] = ColliderComponent_create_rectangle(1.0, 1.0);
    component->physics[i] = PhysicsComponent_create(1.0, 0.5, 0.5, 2.0, 4.0);
}


void create_fire(ComponentData* component, sfVector2f pos) {
    int i = get_index(component);

    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->light[i] = LightComponent_create(10.0, 2.0 * M_PI, 201, sfColor_fromRGB(255, 165, 0), 0.5, 10.0);
    component->particle[i] = ParticleComponent_create(0.5 * M_PI, 1.0, 0.8, 0.2, 1.0, 5.0, sfColor_fromRGB(255, 165, 0), sfColor_fromRGB(255, 255, 0));
    component->particle[i]->loop = true;
    component->particle[i]->enabled = true;
    component->collider[i] = ColliderComponent_create_circle(0.35);
    component->image[i] = ImageComponent_create("fire", 1.0, 1.0, 5);
}


void create_floor(ComponentData* component, sfVector2f pos, float width, float height, float angle) {
    int i = get_index(component);

    component->coordinate[i] = CoordinateComponent_create(pos, angle);
    component->image[i] = ImageComponent_create("board_tile", width, height, 1);
}


void create_house(ComponentData* component, float x, float y) {
    float angle = float_rand(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(3.0, angle);
    sfVector2f h = perp(w);

    sfVector2f pos = { x, y };

    create_floor(component, pos, 6.0, 6.0, angle);

    create_wall(component, sum(pos, w), 5.5, 0.5, angle + 0.5 * M_PI);
    create_wall(component, diff(pos, w), 5.5, 0.5, angle + 0.5 * M_PI);
    create_wall(component, diff(pos, h), 6.5, 0.5, angle);

    create_wall(component, sum(pos, sum(h, mult(2.0 / 3.0, w))), 2.5, 0.5, angle + M_PI);
    create_wall(component, sum(pos, diff(h, mult(2.0 / 3.0, w))), 2.5, 0.5, angle + M_PI);

    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));

    create_fire(component, sum(pos, mult(0.01, w)));

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
    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    //component->collider[i] = ColliderComponent_create_rectangle(1.0, 0.25);
    component->physics[i] = PhysicsComponent_create(0.5, 0.0, 0.5, 10.0, 2.5);
    component->item[i] = ItemComponent_create(0);
    component->light[i] = LightComponent_create(7.0, 1.0, 51, sfColor_fromRGB(255, 255, 150), 0.75, 10.0);
    component->light[i]->enabled = false;
    component->image[i] = ImageComponent_create("flashlight", 1.0, 1.0, 3);
}


void create_level(ComponentData* component) {
    create_house(component, -20.0, 0.0);
    create_house(component, 20.0, 0.0);
    create_house(component, 10.0, 10.0);

    create_player(component, (sfVector2f) { 0.0, 0.0 });

    create_car(component, 0.0, -5.0);
    create_weapon(component, 0.0, 5.0);

    create_flashlight(component, 0.0, 2.0);

    //create_lasersight(component, -2.0, 0.0);
}
