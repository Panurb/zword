#pragma once

#include "component.h"
#include "camera.h"
#include "list.h"


#define CHUNK_WIDTH 32
#define CHUNK_HEIGHT 32
#define LEVEL_WIDTH 9
#define LEVEL_HEIGHT 9


typedef struct {
    int left;
    int right;
    int top;
    int bottom;
} Bounds;

typedef struct {
    // Swap x and y?
    List* array[LEVEL_WIDTH * CHUNK_WIDTH][LEVEL_HEIGHT * CHUNK_HEIGHT];
    int columns;
    int rows;
    int tile_size;
    float tile_width;
    float tile_height;
    float width;
    float height;
} ColliderGrid;

ColliderGrid* ColliderGrid_create();

List* get_entities(ComponentData* components, ColliderGrid* grid, sfVector2f origin, float radius);

void update_grid(ComponentData* component, ColliderGrid* grid, int i);

void init_grid(ComponentData* component, ColliderGrid* grid);

void clear_grid(ComponentData* component, ColliderGrid* grid, int i);

Bounds get_bounds(ComponentData* component, ColliderGrid* grid, int i);

void get_neighbors(ComponentData* component, ColliderGrid* grid, int i, int entities[100]);

void draw_grid(ComponentData* components, ColliderGrid* grid, sfRenderWindow* window, int camera);
