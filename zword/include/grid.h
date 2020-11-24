#pragma once

#include "component.h"

typedef struct {
    int left;
    int right;
    int top;
    int bottom;
} Bounds;

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

Bounds get_bounds(Component* component, ColliderGrid* grid, int i);

void get_neighbors(Component* component, ColliderGrid* grid, int i, int* array, int size);
