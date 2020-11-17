#include <stdlib.h>

#include "image.h"
#include "component.h"


void create_wall(Component* component) {
    int i = component->entities;
    component->entities++;

    CoordinateComponent* coord = malloc(sizeof(CoordinateComponent));
    sfVector2f pos = { 3.0, 0.0 };
    coord->position = pos;
    coord->angle = 0.0;
    component->coordinate[i] = coord;

    ImageComponent* image = malloc(sizeof(ImageComponent));
    sfVector2f scale = { 2, 2 };
    sfSprite* sprite = load_sprite("player", scale);
    image->scale = scale;
    image->sprite = sprite;
    component->image[i] = image;

    CircleColliderComponent* col = malloc(sizeof(CircleColliderComponent));
    col->position = pos;
    col->radius = 1.0;
    col->shape = sfCircleShape_create();
    component->circle_collider[i] = col;
}
