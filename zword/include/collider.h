#pragma once

#include <SFML/System/Vector2.h>

#include "component.h"
#include "camera.h"


typedef struct{
    int array[64][64][10];
    int width;
    int height;
    int size;
    float tile_width;
    float tile_height;
} ColliderGrid;

ColliderGrid* ColliderGrid_create();

void update_grid(Component* component, ColliderGrid* collision_grid, int i);

void clear_grid(Component* component, ColliderGrid* collision_grid, int i);

void collide(Component* component, ColliderGrid* collision_grid);

void debug_draw(Component* component, ColliderGrid* grid, sfRenderWindow* window, Camera* camera);

sfVector2f raycast(Component* component, ColliderGrid* grid, sfVector2f start, sfVector2f velocity, float range);
