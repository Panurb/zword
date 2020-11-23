#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"
#include "camera.h"


typedef struct{
    int array[64][64][10];
    int width;
    int height;
    int size;
} ColliderGrid;

ColliderGrid* ColliderGrid_create();

void update_grid(Component* component, ColliderGrid* collision_grid, int i);

void clear_grid(Component* component, ColliderGrid* collision_grid, int i);

void collide(Component* component, ColliderGrid* collision_grid);

void debug_draw(Component* component, sfRenderWindow* window, Camera* camera);
