#include <stdlib.h>
#include <math.h>

#include "grid.h"
#include "component.h"
#include "collider.h"
#include "util.h"


ColliderGrid* ColliderGrid_create() {
    ColliderGrid* grid = malloc(sizeof(ColliderGrid));
    grid->width = 64;
    grid->height = 64;
    grid->size = 10;
    grid->tile_width = 1.0;
    grid->tile_height = 1.0;

    for (int i = 0; i < grid->width; i++) {
        for (int j = 0; j < grid->height; j++) {
            for (int k = 0; k < grid->size; k++) {
                grid->array[i][j][k] = -1;
            }
        }
    }

    return grid;
}


Bounds get_bounds(ComponentData* component, ColliderGrid* grid, int i) {
    CoordinateComponent* coord = component->coordinate[i];
    ColliderComponent* col = component->collider[i];
    
    int x = floor(coord->position.x + 0.5 * grid->width);
    int y = floor(coord->position.y + 0.5 * grid->height);

    int w;
    int h;
    if (col->type == RECTANGLE) {
        w = ceil(axis_half_width(component, i, (sfVector2f) { 1.0, 0.0 }));
        h = ceil(axis_half_width(component, i, (sfVector2f) { 0.0, 1.0 }));
    } else {
        w = ceil(col->radius);
        h = w;
    }

    Bounds bounds;
    bounds.left = max(0, x - w);
    bounds.right = min(grid->width - 1, x + w);
    bounds.bottom = max(0, y - h);
    bounds.top = min(grid->height - 1, y + h);

    return bounds;
}


void get_entities(ComponentData* components, ColliderGrid* grid, sfVector2f origin, float radius, int entities[100]) {
    int id = 2 * MAX_ENTITIES + 1;

    int x = floor(origin.x + 0.5 * grid->width);
    int y = floor(origin.y + 0.5 * grid->height);
    int rx = radius / grid->tile_width;
    int ry = radius / grid->tile_height;

    int l = 0;
    for (int i = max(0, x - rx); i <= min(grid->width - 1, x + rx); i++) {
        for (int j = max(0, y - ry); j <= min(grid->height - 1, y + ry); j++) {
            for (int k = 0; k < grid->size; k++) {
                int n = grid->array[i][j][k];
                if (n == -1) continue;

                ColliderComponent* col = ColliderComponent_get(components, n);

                if (col->last_collision == id) continue;

                entities[l] = n;
                l++;

                col->last_collision = id;
            }
        }
    }

    for (int i = l; i < 100; i++) {
        entities[i] = -1;
    }
}


void update_grid(ComponentData* component, ColliderGrid* grid, int i) {
    Bounds bounds = get_bounds(component, grid, i);

    for (int j = bounds.left; j <= bounds.right; j++) {
        for (int k = bounds.bottom; k <= bounds.top; k++) {
            for (int l = 0; l < grid->size; l++) {
                if (grid->array[j][k][l] == -1) {
                    grid->array[j][k][l] = i;
                    break;
                }
            }
        }
    }
}


void init_grid(ComponentData* component, ColliderGrid* grid) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->collider[i]) continue;
        
        update_grid(component, grid, i);
    }

}


void clear_grid(ComponentData* component, ColliderGrid* grid, int i) {
    Bounds bounds = get_bounds(component, grid, i);
    
    for (int j = bounds.left; j <= bounds.right; j++) {
        for (int k = bounds.bottom; k <= bounds.top; k++) {
            for (int l = 0; l < grid->size; l++) {
                if (grid->array[j][k][l] == i) {
                    grid->array[j][k][l] = -1;
                    break;
                }
            }
        }
    }
}


void get_neighbors(ComponentData* component, ColliderGrid* grid, int i, int* array, int size) {
    Bounds bounds = get_bounds(component, grid, i);

    for (int j = 0; j < size; j++) {
        array[j] = -1;
    }

    for (int j = bounds.left; j <= bounds.right; j++) {
        for (int k = bounds.bottom; k <= bounds.top; k++) {
            for (int l = 0; l < grid->size; l++) {
                int n = grid->array[j][k][l];

                if (n == -1 || n == i) continue;

                for (int m = 0; m < size; m++) {
                    if (array[m] == n) break;

                    if (array[m] == -1) {
                        array[m] = n;
                        break;
                    }
                }
            }
        }
    }
}
