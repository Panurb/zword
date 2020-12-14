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

void get_entities(ComponentData* components, ColliderGrid* grid, sfVector2f origin, float radius, int entities[100]);

void update_grid(ComponentData* component, ColliderGrid* grid, int i);

void init_grid(ComponentData* component, ColliderGrid* grid);

void clear_grid(ComponentData* component, ColliderGrid* grid, int i);

Bounds get_bounds(ComponentData* component, ColliderGrid* grid, int i);

void get_neighbors(ComponentData* component, ColliderGrid* grid, int i, int* array, int size);
