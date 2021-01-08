#include <stdlib.h>
#include <math.h>

#include "grid.h"
#include "component.h"
#include "collider.h"
#include "util.h"
#include "camera.h"


ColliderGrid* ColliderGrid_create() {
    ColliderGrid* grid = malloc(sizeof(ColliderGrid));
    grid->columns = 6 * 32;
    grid->rows = 6 * 32;
    grid->tile_size = 10;
    grid->tile_width = 1.0;
    grid->tile_height = 1.0;
    grid->width = grid->columns * grid->tile_width;
    grid->height = grid->rows * grid->tile_height;

    for (int i = 0; i < grid->columns; i++) {
        for (int j = 0; j < grid->rows; j++) {
            for (int k = 0; k < grid->tile_size; k++) {
                grid->array[i][j][k] = -1;
            }
        }
    }

    return grid;
}


Bounds get_bounds(ComponentData* components, ColliderGrid* grid, int i) {
    sfVector2f pos = get_position(components, i);
    
    float x = (pos.x + 0.5 * grid->width) / grid->tile_width;
    float y = (pos.y + 0.5 * grid->height) / grid->tile_height;

    float w = axis_half_width(components, i, (sfVector2f) { 1.0, 0.0 }) / grid->tile_width;
    float h = axis_half_width(components, i, (sfVector2f) { 0.0, 1.0 }) / grid->tile_height;

    Bounds bounds;
    bounds.left = max(0, floorf(x - w));
    bounds.right = min(grid->columns - 1, floorf(x + w));
    bounds.bottom = max(0, floorf(y - h));
    bounds.top = min(grid->rows - 1, floorf(y + h));

    return bounds;
}


void get_entities(ComponentData* components, ColliderGrid* grid, sfVector2f origin, float radius, int entities[100]) {
    static int id = 2 * MAX_ENTITIES;
    id = (id < 3 * MAX_ENTITIES) ? id + 1 : 2 * MAX_ENTITIES;

    float x = (origin.x + 0.5 * grid->width) / grid->tile_width;
    float y = (origin.y + 0.5 * grid->height) / grid->tile_height;
    float r = radius / grid->tile_width;

    int l = 0;
    for (int i = max(0, x - r); i <= min(grid->columns - 1, x + r); i++) {
        for (int j = max(0, y - r); j <= min(grid->rows - 1, y + r); j++) {
            for (int k = 0; k < grid->tile_size; k++) {
                int n = grid->array[i][j][k];
                if (n == -1) continue;
                if (dist(origin, get_position(components, n)) > radius) continue;

                ColliderComponent* col = ColliderComponent_get(components, n);
                if (col->last_collision == id) continue;

                entities[l] = n;
                l++;

                col->last_collision = id;
            }
        }
    }

    entities[l] = -1;
}


void update_grid(ComponentData* components, ColliderGrid* grid, int entity) {
    Bounds bounds = get_bounds(components, grid, entity);

    for (int i = bounds.left; i <= bounds.right; i++) {
        for (int j = bounds.bottom; j <= bounds.top; j++) {
            for (int k = 0; k < grid->tile_size; k++) {
                if (grid->array[i][j][k] == -1) {
                    grid->array[i][j][k] = entity;
                    break;
                }
            }
        }
    }
}


void init_grid(ComponentData* components, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        if (ColliderComponent_get(components, i)) {
            update_grid(components, grid, i);
        }
    }
}


void clear_grid(ComponentData* components, ColliderGrid* grid, int entity) {
    Bounds bounds = get_bounds(components, grid, entity);
    
    for (int i = bounds.left; i <= bounds.right; i++) {
        for (int j = bounds.bottom; j <= bounds.top; j++) {
            for (int k = 0; k < grid->tile_size; k++) {
                if (grid->array[i][j][k] == entity) {
                    grid->array[i][j][k] = -1;
                    break;
                }
            }
        }
    }
}


void get_neighbors(ComponentData* components, ColliderGrid* grid, int entity, int entities[100]) {
    int id = 2 * MAX_ENTITIES + 2;

    Bounds bounds = get_bounds(components, grid, entity);

    int l = 0;
    for (int i = bounds.left; i <= bounds.right; i++) {
        for (int j = bounds.bottom; j <= bounds.top; j++) {
            for (int k = 0; k < grid->tile_size; k++) {
                int n = grid->array[i][j][k];
                if (n == -1 || n == entity) continue;

                ColliderComponent* col = ColliderComponent_get(components, n);
                if (col->last_collision == id) continue;

                entities[l] = n;
                l++;

                col->last_collision = id;
            }
        }
    }
}


void draw_grid(ColliderGrid* grid, sfRenderWindow* window, Camera* camera) {
    sfColor color = get_color(1.0, 1.0, 1.0, 0.2);

    int nx = ceil((camera->resolution.x / camera->zoom) / grid->tile_width) + 1;
    float x = floor(camera->position.x - 0.5 * camera->resolution.x / camera->zoom);
    for (int i = 0; i < nx; i++) {
        sfVector2f start = { x + i * grid->tile_width, camera->position.y + 0.5 * camera->resolution.y / camera->zoom };
        sfVector2f end = { x + i * grid->tile_width, camera->position.y - 0.5 * camera->resolution.y / camera->zoom };

        draw_line(window, camera, NULL, start, end, 0.02, color);
    }

    int ny = ceil((camera->resolution.y / camera->zoom) / grid->tile_height);    
    float y = floor(camera->position.y - 0.5 * camera->resolution.y / camera->zoom);
    for (int i = 0; i < ny; i++) {
        sfVector2f start = { camera->position.x - 0.5 * camera->resolution.x / camera->zoom, y + i * grid->tile_height };
        sfVector2f end = { camera->position.x + 0.5 * camera->resolution.x / camera->zoom, y + i * grid->tile_height };

        draw_line(window, camera, NULL, start, end, 0.02, color);
    }
}
