#include <stdlib.h>
#include <math.h>

#include "grid.h"
#include "component.h"
#include "collider.h"
#include "util.h"
#include "camera.h"
#include "level.h"
#include "list.h"


ColliderGrid* ColliderGrid_create() {
    ColliderGrid* grid = malloc(sizeof(ColliderGrid));
    grid->columns = LEVEL_WIDTH * CHUNK_WIDTH;
    grid->rows = LEVEL_HEIGHT * CHUNK_HEIGHT;
    grid->tile_size = 10;
    grid->tile_width = 1.0;
    grid->tile_height = 1.0;
    grid->width = grid->columns * grid->tile_width;
    grid->height = grid->rows * grid->tile_height;

    for (int i = 0; i < grid->columns; i++) {
        for (int j = 0; j < grid->rows; j++) {
            grid->array[i][j] = List_create();
        }
    }

    return grid;
}


void ColliderGrid_clear(ColliderGrid* grid) {
    for (int i = 0; i < grid->columns; i++) {
        for (int j = 0; j < grid->rows; j++) {
            List_clear(grid->array[i][j]);
        }
    }
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


List* get_entities(ComponentData* components, ColliderGrid* grid, sfVector2f origin, float radius) {
    List* list = List_create();

    static int id = 2 * MAX_ENTITIES;
    id = (id < 3 * MAX_ENTITIES) ? id + 1 : 2 * MAX_ENTITIES;

    float x = (origin.x + 0.5f * grid->width) / grid->tile_width;
    float y = (origin.y + 0.5f * grid->height) / grid->tile_height;
    float r = radius / grid->tile_width;

    for (int i = max(0, x - r); i <= min(grid->columns - 1, x + r); i++) {
        for (int j = max(0, y - r); j <= min(grid->rows - 1, y + r); j++) {
            for (ListNode* node = grid->array[i][j]->head; node; node = node->next) {
                int n = node->value;

                ColliderComponent* col = ColliderComponent_get(components, n);
                if (col->last_collision == id) continue;

                if (dist(origin, get_position(components, n)) <= radius) {
                    List_add(list, n);
                }

                col->last_collision = id;
            }
        }
    }

    return list;
}


void update_grid(ComponentData* components, ColliderGrid* grid, int entity) {
    Bounds bounds = get_bounds(components, grid, entity);

    for (int i = bounds.left; i <= bounds.right; i++) {
        for (int j = bounds.bottom; j <= bounds.top; j++) {
            List_add(grid->array[i][j], entity);
        }
    }

    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    for (ListNode* node = coord->children->head; node; node = node->next) {
        if (ColliderComponent_get(components, node->value)) {
            update_grid(components, grid, node->value);
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
            List_remove(grid->array[i][j], entity);
        }
    }

    CoordinateComponent* coord = CoordinateComponent_get(components, entity);
    for (ListNode* node = coord->children->head; node; node = node->next) {
        if (ColliderComponent_get(components, node->value)) {
            clear_grid(components, grid, node->value);
        }
    }
}


void get_neighbors(ComponentData* components, ColliderGrid* grid, int entity, int entities[100]) {
    int id = 2 * MAX_ENTITIES + 2;

    Bounds bounds = get_bounds(components, grid, entity);

    int l = 0;
    for (int i = bounds.left; i <= bounds.right; i++) {
        for (int j = bounds.bottom; j <= bounds.top; j++) {
            for (ListNode* current = grid->array[i][j]->head; current != NULL; current = current->next) {
                int n = current->value;
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


void draw_grid(ComponentData* components, ColliderGrid* grid, sfRenderWindow* window, int camera) {
    CameraComponent* cam = CameraComponent_get(components, camera);
    sfVector2f pos = CoordinateComponent_get(components, camera)->position;

    sfColor color = get_color(1.0, 1.0, 1.0, 0.2);

    int nx = ceil((cam->resolution.x / cam->zoom) / grid->tile_width) + 1;
    float x = floor(pos.x - 0.5 * cam->resolution.x / cam->zoom);
    for (int i = 0; i < nx; i++) {
        sfVector2f start = { x + i * grid->tile_width, pos.y + 0.5 * cam->resolution.y / cam->zoom };
        sfVector2f end = { x + i * grid->tile_width, pos.y - 0.5 * cam->resolution.y / cam->zoom };

        draw_line(window, components, camera, NULL, start, end, 0.02, color);
    }

    int ny = ceil((cam->resolution.y / cam->zoom) / grid->tile_height);    
    float y = floor(pos.y - 0.5 * cam->resolution.y / cam->zoom);
    for (int i = 0; i < ny; i++) {
        sfVector2f start = { pos.x - 0.5 * cam->resolution.x / cam->zoom, y + i * grid->tile_height };
        sfVector2f end = { pos.x + 0.5 * cam->resolution.x / cam->zoom, y + i * grid->tile_height };

        draw_line(window, components, camera, NULL, start, end, 0.02, color);
    }
}
