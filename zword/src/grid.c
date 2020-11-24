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


Bounds get_bounds(Component* component, ColliderGrid* grid, int i) {
    CoordinateComponent* coord = component->coordinate[i];
    
    int x = floor(coord->position.x + 0.5 * grid->width);
    int y = floor(coord->position.y + 0.5 * grid->height);

    int w;
    int h;
    if (component->rectangle_collider[i]) {
        w = ceil(axis_half_width(component, i, (sfVector2f) { 1.0, 0.0 }));
        h = ceil(axis_half_width(component, i, (sfVector2f) { 0.0, 1.0 }));
    } else if (component->circle_collider[i]) {
        w = ceil(component->circle_collider[i]->radius);
        h = w;
    }

    Bounds bounds;
    bounds.left = max(0, x - w);
    bounds.right = min(grid->width - 1, x + w);
    bounds.bottom = max(0, y - h);
    bounds.top = min(grid->height - 1, y + h);

    return bounds;
}


void update_grid(Component* component, ColliderGrid* grid, int i) {
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


void clear_grid(Component* component, ColliderGrid* grid, int i) {
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


void get_neighbors(Component* component, ColliderGrid* grid, int i, int* array, int size) {
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
