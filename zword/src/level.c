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


void create_waypoint(Component* component, sfVector2f pos) {
    int i = get_index(component);

    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->waypoint[i] = WaypointComponent_create();
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
}


void create_wall(Component* component, sfVector2f pos, float width, float height, float angle) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, angle);
    component->rectangle_collider[i] = RectangleColliderComponent_create(width, height);
}


void create_prop(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->rectangle_collider[i] = RectangleColliderComponent_create(1.0, 1.0);
    component->physics[i] = PhysicsComponent_create(1.0, 0.5, 0.5, 2.0, 4.0);
}


void create_fire(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->light[i] = LightComponent_create(10.0, 2.0 * M_PI, 201, sfColor_fromRGB(255, 165, 0), 0.25, 10.0);
    component->particle[i] = ParticleComponent_create(0.5 * M_PI, 1.0, 1.0, 0.2, 1.0, 5.0, sfColor_fromRGB(255, 165, 0), sfColor_fromRGB(255, 255, 0));
    component->particle[i]->loop = true;
    component->particle[i]->enabled = true;
    component->circle_collider[i] = CircleColliderComponent_create(0.35);
}


void create_house(Component* component, float x, float y) {
    float angle = float_rand(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(3.0, angle);
    sfVector2f h = perp(w);

    sfVector2f pos = { x, y };

    create_wall(component, sum(pos, w), 0.5, 5.5, angle);
    create_wall(component, diff(pos, w), 0.5, 5.5, angle);
    create_wall(component, diff(pos, h), 6.5, 0.5, angle + M_PI);

    create_wall(component, sum(pos, sum(h, mult(2.0 / 3.0, w))), 2.5, 0.5, angle + M_PI);
    create_wall(component, sum(pos, diff(h, mult(2.0 / 3.0, w))), 2.5, 0.5, angle + M_PI);

    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_enemy(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));

    create_fire(component, pos);

    create_waypoint(component, sum(pos, mult(0.75, h)));
    create_waypoint(component, sum(pos, mult(1.5, h)));
    create_waypoint(component, sum(pos, mult(1.5, sum(w, h))));
    create_waypoint(component, sum(pos, mult(1.5, diff(w, h))));
    create_waypoint(component, sum(pos, mult(-1.5, sum(w, h))));
    create_waypoint(component, sum(pos, mult(-1.5, diff(w, h))));
}


void create_flashlight(Component* component, float x, float y) {
    int i = component->entities;
    component->entities++;

    sfVector2f pos = { x, y };
    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->rectangle_collider[i] = RectangleColliderComponent_create(1.0, 0.25);
    component->physics[i] = PhysicsComponent_create(0.5, 0.0, 0.5, 10.0, 2.5);
    component->item[i] = ItemComponent_create(1);
    component->light[i] = LightComponent_create(7.0, 1.0, 51, sfColor_fromRGB(255, 255, 150), 0.75, 10.0);
    component->light[i]->enabled = false;
}


void create_level(Component* component) {
    create_house(component, -20.0, 0.0);
    create_house(component, 20.0, 0.0);
    create_house(component, 10.0, 10.0);

    create_player(component, (sfVector2f) { 0.0, 0.0 });

    create_car(component, 0.0, -5.0);
    create_weapon(component, 0.0, 5.0);

    create_flashlight(component, 0.0, 2.0);
}
