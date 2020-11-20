#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>

#include "image.h"
#include "component.h"
#include "player.h"
#include "util.h"


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
    //component->rectangle_collider[i] = RectangleColliderComponent_create(width, height);
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
    component->physics[i] = PhysicsComponent_create(1.0, 0.5, 0.5, 2.0);
}


void create_house(Component* component, float x, float y) {
    float angle = float_rand(0.0, 2 * M_PI);

    sfVector2f w = polar_to_cartesian(3.0, angle);
    sfVector2f h = perp(w);

    sfVector2f pos = { x, y };

    create_wall(component, sum(pos, w), 0.5, 6.0, angle);
    create_wall(component, diff(pos, w), 0.5, 6.0, angle);
    create_wall(component, diff(pos, h), 6.0, 0.5, angle + M_PI);

    create_wall(component, sum(pos, sum(h, mult(2.0 / 3.0, w))), 2.0, 0.5, angle + M_PI);
    create_wall(component, sum(pos, diff(h, mult(2.0 / 3.0, w))), 2.0, 0.5, angle + M_PI);

    create_prop(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_prop(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
    create_prop(component, sum(pos, polar_to_cartesian(2.0, float_rand(0.0, 2 * M_PI))));
}


void create_level(Component* component) {
    create_house(component, 0.0, 0.0);
    create_house(component, 20.0, 0.0);
    create_house(component, 10.0, 10.0);

    create_player(component, (sfVector2f) { 0.0, 0.0 });
}
