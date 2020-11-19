#include <stdlib.h>

#include "image.h"
#include "component.h"
#include "player.h"


void create_wall(Component* component, float x, float y, float width, float height, float angle) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(x, y, angle);
    component->rectangle_collider[i] = RectangleColliderComponent_create(width, height);
}


void create_prop(Component* component, float x, float y, float width, float height, float angle) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(x, y, angle);
    //component->rectangle_collider[i] = RectangleColliderComponent_create(width, height);
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
    component->physics[i] = PhysicsComponent_create(1.0, 0.5, 0.5);
}


void create_level(Component* component) {
    create_wall(component, 3.0, 0.0, 0.5, 6.0, 0.0);
    create_wall(component, -3.0, 0.0, 0.5, 6.0, 0.0);
    create_wall(component, 0.0, -3.0, 6.0, 0.5, 0.0);
    create_wall(component, 0.0, 3.0, 6.0, 0.5, 0.0);

    create_prop(component, 0.0, 0.0, 1.0, 1.0, 0.4);
    create_prop(component, 1.0, 1.0, 1.0, 1.0, 0.4);
    create_prop(component, 0.5, -1.0, 1.0, 1.0, 0.4);
    create_prop(component, 2.0, -2.0, 1.0, 1.0, 0.4);

    create_player(component, 2.0, 0.0);
}
