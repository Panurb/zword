#include <stdlib.h>

#include "image.h"
#include "component.h"


void create_wall(Component* component, float x, float y, float width, float height, float angle) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(x, y, angle);
    component->rectangle_collider[i] = RectangleColliderComponent_create(width, height);
}
