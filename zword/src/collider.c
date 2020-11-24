#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>

#include "collider.h"
#include "component.h"
#include "camera.h"
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


sfVector2f half_width(Component* component, int i) {
    return polar_to_cartesian(0.5 * component->rectangle_collider[i]->width, component->coordinate[i]->angle);
}


sfVector2f half_height(Component* component, int i) {
    return polar_to_cartesian(0.5 * component->rectangle_collider[i]->height, component->coordinate[i]->angle + 0.5 * M_PI);
}


float axis_half_width(Component* component, int i, sfVector2f axis) {
    sfVector2f hw = half_width(component, i);
    sfVector2f hh = half_height(component, i);
    return fabs(dot(hw, axis)) + fabs(dot(hh, axis));
}


float axis_overlap(float w1, sfVector2f r1, float w2, sfVector2f r2, sfVector2f axis) {
    float r = dot(diff(r1, r2), axis);
    float o = w1 + w2 - fabs(r);
    if (o > 0.0) {
        if (fabs(r) < 1e-6) {
            return o;
        } else {
            return copysignf(o, r);
        }
    }

    return 0.0;
}

sfVector2f overlap_circle_circle(Component* component, int i, int j) {
    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;

    float d = dist(a, b);
    float r = component->circle_collider[i]->radius + component->circle_collider[j]->radius;

    if (d > r) {
        return (sfVector2f) { 0.0, 0.0 };
    }

    if (d < 1e-6) {
        return (sfVector2f) { 0.0, r };
    }

    return mult(r / d - 1, diff(a, b));
}

sfVector2f overlap_rectangle_circle(Component* component, int i, int j) {
    bool near_corner = true;

    float overlaps[2] = { 0.0, 0.0 };
    sfVector2f axes[2];
    axes[0] = polar_to_cartesian(1.0, component->coordinate[i]->angle);
    axes[1] = perp(axes[0]);

    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;
    float radius = component->circle_collider[j]->radius;

    for (int k = 0; k < 2; k++) {
        overlaps[k] = axis_overlap(axis_half_width(component, i, axes[k]), a, radius, b, axes[k]);

        if (fabs(overlaps[k]) < 1e-6) {
            return (sfVector2f) { 0.0, 0.0 };
        }

        if (fabs(overlaps[k]) >= radius) {
            near_corner = false;
        }
    }

    int k = abs_argmin(overlaps, 2);
    if (!near_corner) {
        return mult(overlaps[k], axes[k]);
    }

    sfVector2f hw = half_width(component, i);
    sfVector2f hh = half_height(component, i);

    sfVector2f corner = diff(a, sum(mult(sign(overlaps[0]), hw), mult(sign(overlaps[1]), hh)));

    sfVector2f axis = normalized(diff(corner, b));

    float overlap = axis_overlap(axis_half_width(component, i, axis), a, radius, b, axis);

    if (0.0 < fabs(overlap) && fabs(overlap) < fabs(overlaps[k])) {
        return mult(overlap, axis);
    }

    return (sfVector2f) { 0.0, 0.0 };
}

sfVector2f overlap_circle_rectangle(Component* component, int i, int j) {
    return mult(-1.0, overlap_rectangle_circle(component, j, i));
}

sfVector2f overlap_rectangle_rectangle(Component* component, int i, int j) {
    sfVector2f hw_i = polar_to_cartesian(1.0, component->coordinate[i]->angle);
    sfVector2f hw_j = polar_to_cartesian(1.0, component->coordinate[j]->angle);

    float overlaps[4];
    sfVector2f axes[4] = { hw_i, perp(hw_i), hw_j, perp(hw_j) };

    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;

    for (int k = 0; k < 4; k++) {
        overlaps[k] = axis_overlap(axis_half_width(component, i, axes[k]), a,
                                   axis_half_width(component, j, axes[k]), b, axes[k]);

        if (fabs(overlaps[k]) < 1e-6) {
            return (sfVector2f) { 0.0, 0.0 };
        }
    }

    int k = abs_argmin(overlaps, 4);

    return mult(overlaps[k], axes[k]);
}

sfVector2f overlap(Component* component, int i, int j) {
    sfVector2f ol = { 0.0, 0.0 };
    if (component->circle_collider[i]) {
        if (component->circle_collider[j]) {
            ol = overlap_circle_circle(component, i, j);
        } else if (component->rectangle_collider[j]) {
            ol = overlap_circle_rectangle(component, i, j);
        }
    } else if (component->rectangle_collider[i]) {
        if (component->circle_collider[j]) {
            ol = overlap_rectangle_circle(component, i, j);
        } else if (component->rectangle_collider[j]) {
            ol = overlap_rectangle_rectangle(component, i, j);
        }
    }

    return ol;
}


typedef struct {
    int left;
    int right;
    int top;
    int bottom;
} Bounds;


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


sfVector2f raycast(Component* component, ColliderGrid* grid, sfVector2f start, sfVector2f velocity, float range) {
    //http://www.cs.yorku.ca/~amana/research/grid.pdf

    velocity = normalized(velocity);

    int x = floor(start.x / grid->tile_width + 0.5 * grid->width);
    int y = floor(start.y / grid->tile_height + 0.5 * grid->height);

    int step_x = sign(velocity.x);
    int step_y = sign(velocity.y);

    float offset_x = mod(step_x * start.x, grid->tile_width);
    float offset_y = mod(step_y * start.y, grid->tile_height);

    float t_max_x = (grid->tile_width - offset_x) / fabs(velocity.x);
    float t_max_y = (grid->tile_height - offset_y) / fabs(velocity.y);

    float t_delta_x = grid->tile_width / fabs(velocity.x);
    float t_delta_y = grid->tile_height / fabs(velocity.y);
    
    float t_min = range;
    sfVector2f point = sum(start, mult(range, velocity));
    while (1) {
        if (t_max_x < t_max_y) {
            t_max_x += t_delta_x;
            x += step_x;
        } else {
            t_max_y += t_delta_y;
            y += step_y;
        }

        if (min(t_max_x, t_max_y) > range) {
            break;
        }

        if (x < 0 || x > grid->width - 1) {
            break;
        }

        if (y < 0 || y > grid->height - 1) {
            break;
        }

        for (int jj = 0; jj < grid->size; jj++) {
            int j = grid->array[x][y][jj];
            if (j == -1)  continue;

            CoordinateComponent* coord = component->coordinate[j];
            if (component->rectangle_collider[j]) {
                //https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282

                sfVector2f hw = half_width(component, j);
                sfVector2f hh = half_height(component, j);
                sfVector2f corners[4];
                corners[0] = sum(coord->position, sum(hw, hh));
                corners[1] = sum(coord->position, diff(hw, hh));
                corners[2] = diff(coord->position, sum(hw, hh));
                corners[3] = diff(coord->position, diff(hw, hh));

                for (int k = 0; k < 4; k++) {
                    sfVector2f dir = diff(corners[(k + 1) % 4], corners[k]);
                    float t = cross(diff(corners[k], start), dir) / cross(velocity, dir);
                    float u = cross(diff(start, corners[k]), velocity) / cross(dir, velocity);

                    if (t >= 0.0 && t < t_min && u >= 0.0 && u <= 1.0) {
                        point = sum(start, mult(t, velocity));
                        t_min = t;
                    }
                }
            } else if (component->circle_collider[j]) {
                //https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

                float radius = component->circle_collider[j]->radius;
                sfVector2f oc = diff(start, coord->position);
                float delta = powf(dot(velocity, oc), 2) - norm2(oc) + powf(radius, 2);
                float t = -dot(velocity, oc) - sqrtf(delta);

                if (delta >= 0.0 && t >= 0.0) {
                    if (t < t_min) {
                        point = sum(start, mult(t, velocity));
                        t_min = t;
                    }
                }
            }
        }
    }

    return point;
}


void collide(Component* component, ColliderGrid* grid) {
    for (int i = 0; i < component->entities; i++) {
        PhysicsComponent* physics = component->physics[i];
        if (!physics) continue;

        int neighbors[20];
        get_neighbors(component, grid, i, neighbors, 20);

        for (int j = 0; j < 20; j++) {
            int k = neighbors[j];

            if (k == -1) break;

            sfVector2f ol = overlap(component, i, k);

            if (ol.x == 0.0 && ol.y == 0.0) continue;

            float m = 1.0;
            sfVector2f other_vel = { 0.0, 0.0 };
            if (component->physics[k]) {
                m = component->physics[k]->mass / (physics->mass + component->physics[k]->mass);
                other_vel = component->physics[k]->velocity;
            }

            sfVector2f dv = diff(physics->velocity, other_vel);
            sfVector2f n = normalized(ol);
            sfVector2f new_vel = diff(physics->velocity, mult(2 * m * dot(dv, n), n));

            physics->collision.velocity = sum(physics->collision.velocity, new_vel);

            physics->collision.overlap = sum(physics->collision.overlap, mult(m, ol));
        }
    }
}


void debug_draw(Component* component, ColliderGrid* grid, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        /*
        Bounds bounds = get_bounds(component, grid, i);
        for (int j = bounds.left; j <= bounds.right; j++) {
            for (int k = bounds.bottom; k <= bounds.top; k++) {
                for (int l = 0; l < 10; l++) {
                    sfRectangleShape* shape = sfRectangleShape_create();
                    sfRectangleShape_setOrigin(shape, (sfVector2f) { 0.5 * camera->zoom, 0.5 * camera->zoom });

                    sfVector2f pos = { j - 31.3, k - 31.3 };
                    sfRectangleShape_setPosition(shape, world_to_screen(pos, camera));

                    sfVector2f size = { camera->zoom, camera->zoom };
                    sfRectangleShape_setSize(shape, size);

                    sfRectangleShape_setOutlineColor(shape, sfWhite);
                    sfRectangleShape_setOutlineThickness(shape, 0.1 * camera->zoom);
                    sfRectangleShape_setFillColor(shape, sfTransparent);

                    sfRenderWindow_drawRectangleShape(window, shape, NULL);

                    sfRectangleShape_destroy(shape);
                }
            }
        }
        */

        CoordinateComponent* coord = component->coordinate[i];

        if (component->circle_collider[i]) {
            CircleColliderComponent* col = component->circle_collider[i];

            sfCircleShape_setOrigin(col->shape, (sfVector2f) { col->radius * camera->zoom, col->radius * camera->zoom });

            sfVector2f pos = component->coordinate[i]->position;
            sfCircleShape_setPosition(col->shape, world_to_screen(pos, camera));

            sfCircleShape_setRadius(col->shape, col->radius * camera->zoom);

            sfRenderWindow_drawCircleShape(window, col->shape, NULL);
        } else if (component->rectangle_collider[i]) {
            RectangleColliderComponent* col = component->rectangle_collider[i];

            sfRectangleShape_setOrigin(col->shape, (sfVector2f) { 0.5 * col->width * camera->zoom, 0.5 * col->height * camera->zoom });

            sfVector2f pos = component->coordinate[i]->position;
            sfRectangleShape_setPosition(col->shape, world_to_screen(pos, camera));

            sfVector2f size = { col->width * camera->zoom, col->height * camera->zoom };
            sfRectangleShape_setSize(col->shape, size);

            sfRectangleShape_setRotation(col->shape, -to_degrees(component->coordinate[i]->angle));

            sfRenderWindow_drawRectangleShape(window, col->shape, NULL);
        }
    }
}
