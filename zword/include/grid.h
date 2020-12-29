#pragma once

#include "component.h"
#include "camera.h"


typedef struct {
    int left;
    int right;
    int top;
    int bottom;
} Bounds;

typedef struct {
    int array[6 * 32][6 * 32][10];
    int columns;
    int rows;
    int tile_size;
    float tile_width;
    float tile_height;
    float width;
    float height;
} ColliderGrid;

ColliderGrid* ColliderGrid_create();

void get_entities(ComponentData* components, ColliderGrid* grid, sfVector2f origin, float radius, int entities[100], int entity);

void update_grid(ComponentData* component, ColliderGrid* grid, int i);

void init_grid(ComponentData* component, ColliderGrid* grid);

void clear_grid(ComponentData* component, ColliderGrid* grid, int i);

Bounds get_bounds(ComponentData* component, ColliderGrid* grid, int i);

void get_neighbors(ComponentData* component, ColliderGrid* grid, int i, int entities[100]);

void draw_grid(ColliderGrid* grid, sfRenderWindow* window, Camera* camera);
