#include <stdlib.h>
#include <math.h>
#include <stdio.h>

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
    grid->tile_width = TILE_WIDTH;
    grid->tile_height = TILE_HEIGHT;
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


sfVector2i world_to_grid(ColliderGrid* grid, sfVector2f position) {
    int x = floorf((position.x + 0.5 * grid->width) / grid->tile_width);
    int y = floorf((position.y + 0.5 * grid->height) / grid->tile_height);
    
    return (sfVector2i) { x, y };
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
}


void init_grid(ComponentData* components, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        ColliderComponent* collider = ColliderComponent_get(components, i);
        if (collider && collider->enabled) {
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


void draw_grid(ComponentData* components, sfRenderWindow* window, int camera, float tile_width, float tile_height) {
    CameraComponent* cam = CameraComponent_get(components, camera);
    float width = cam->resolution.x / cam->zoom;
    float height = cam->resolution.y / cam->zoom;
    sfVector2f pos = get_position(components, camera);
    float linewidth = 0.01f;
    float left = pos.x - 0.5f * width;
    float right = pos.x + 0.5f * width;
    float bottom = pos.y - 0.5f * height;
    float top = pos.y + 0.5f * height;
    sfColor color = get_color(1.0f, 1.0f, 1.0f, 0.25f);

    sfRectangleShape* shape = sfRectangleShape_create();
    
    for (float x = left - mod(left, tile_width); x < right; x += tile_width) {
        draw_line(window, components, camera, NULL, vec(x, bottom), vec(x, top), linewidth, color);
    }

    for (float y = left - mod(left, tile_height); y < right; y += tile_height) {
        draw_line(window, components, camera, NULL, vec(left, y), vec(right, y), linewidth, color);
    }
    
    sfRectangleShape_destroy(shape);
}


sfVector2f snap_to_grid(sfVector2f vector) {
    vector.x = TILE_WIDTH * roundf(vector.x / TILE_WIDTH);
    vector.y = TILE_HEIGHT * roundf(vector.y / TILE_HEIGHT);
    return vector;
}


sfVector2f snap_to_grid_center(sfVector2f vector) {
    vector.x = TILE_WIDTH * roundf((vector.x + 0.5f) / TILE_WIDTH) - 0.5f * TILE_WIDTH;
    vector.y = TILE_HEIGHT * roundf((vector.y + 0.5f) / TILE_HEIGHT) - 0.5f * TILE_HEIGHT;
    return vector;
}
