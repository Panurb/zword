#include <stdlib.h>

#include "image.h"
#include "component.h"


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
    component->physics[i] = PhysicsComponent_create(0.0, 1.0);
}
