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
#include "grid.h"
#include "physics.h"


static int COLLISION_MATRIX[7][7] = { { 0, 0, 0, 0, 0, 0, 0 },    // WALLS
                                      { 1, 1, 0, 0, 0, 2, 0 },    // ITEMS
                                      { 1, 0, 2, 2, 1, 2, 0 },    // PLAYERS
                                      { 1, 0, 2, 2, 1, 2, 0 },    // ENEMIES
                                      { 1, 0, 0, 0, 1, 1, 3 },    // VEHICLES
                                      { 0, 0, 0, 0, 0, 0, 0 },    // TREES
                                      { 0, 0, 0, 0, 0, 0, 0 } };  // ROADS


void get_corners(ComponentData* component, int i, sfVector2f* corners) {
    CoordinateComponent* coord = component->coordinate[i];

    sfVector2f hw = half_width(component, i);
    sfVector2f hh = half_height(component, i);

    corners[0] = sum(coord->position, sum(hw, hh));
    corners[1] = diff(corners[0], mult(2, hh));
    corners[2] = diff(corners[1], mult(2, hw));
    corners[3] = sum(corners[2], mult(2, hh));
}


sfVector2f half_width(ComponentData* component, int i) {
    return polar_to_cartesian(0.5 * component->collider[i]->width, component->coordinate[i]->angle);
}


sfVector2f half_height(ComponentData* component, int i) {
    return polar_to_cartesian(0.5 * component->collider[i]->height, component->coordinate[i]->angle + 0.5 * M_PI);
}


float axis_half_width(ComponentData* component, int i, sfVector2f axis) {
    ColliderComponent* col = component->collider[i];

    if (col->type == RECTANGLE) {
        sfVector2f hw = half_width(component, i);
        sfVector2f hh = half_height(component, i);
        return fabs(dot(hw, axis)) + fabs(dot(hh, axis));
    } else {
        return col->radius;
    }
    return 0.0;
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


sfVector2f overlap_circle_circle(ComponentData* component, int i, int j) {
    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;

    float d = dist(a, b);
    float r = component->collider[i]->radius + component->collider[j]->radius;

    if (d > r) {
        return (sfVector2f) { 0.0, 0.0 };
    }

    if (d == 0.0) {
        return (sfVector2f) { 0.0, r };
    }

    return mult(r / d - 1, diff(a, b));
}


sfVector2f overlap_rectangle_circle(ComponentData* component, int i, int j) {
    bool near_corner = true;

    float overlaps[2] = { 0.0, 0.0 };
    sfVector2f axes[2];
    axes[0] = polar_to_cartesian(1.0, component->coordinate[i]->angle);
    axes[1] = perp(axes[0]);

    sfVector2f a = component->coordinate[i]->position;
    sfVector2f b = component->coordinate[j]->position;
    float radius = component->collider[j]->radius;

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

sfVector2f overlap_circle_rectangle(ComponentData* component, int i, int j) {
    return mult(-1.0, overlap_rectangle_circle(component, j, i));
}

sfVector2f overlap_rectangle_rectangle(ComponentData* component, int i, int j) {
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


sfVector2f overlap(ComponentData* component, int i, int j) {
    sfVector2f ol = { 0.0, 0.0 };

    ColliderComponent* a = component->collider[i];
    ColliderComponent* b = component->collider[j];

    if (!COLLISION_MATRIX[a->group][b->group]) return ol;

    if (!a->enabled || !b->enabled) {
        return ol;
    }

    if (a->type == CIRCLE) {
        if (b->type == CIRCLE) {
            ol = overlap_circle_circle(component, i, j);
        } else {
            ol = overlap_circle_rectangle(component, i, j);
        }
    } else {
        if (b->type == CIRCLE) {
            ol = overlap_rectangle_circle(component, i, j);
        } else {
            ol = overlap_rectangle_rectangle(component, i, j);
        }
    }

    return ol;
}


void collide(ComponentData* components, ColliderGrid* grid) {
    // https://en.wikipedia.org/wiki/Elastic_collision#Two-dimensional

    for (int i = 0; i < components->entities; i++) {
        ColliderComponent* collider = ColliderComponent_get(components, i);
        if (!collider) continue;

        collider->last_collision = -1;
    }

    for (int i = 0; i < components->entities; i++) {
        if (!ColliderComponent_get(components, i)) continue;

        PhysicsComponent* physics = components->physics[i];
        if (!physics) continue;

        Bounds bounds = get_bounds(components, grid, i);

        for (int j = bounds.left; j <= bounds.right; j++) {
            for (int k = bounds.bottom; k <= bounds.top; k++) {
                for (int l = 0; l < grid->tile_size; l++) {
                    int n = grid->array[j][k][l];

                    if (n == -1) continue;
                    if (n == i) continue;
                    ColliderComponent* collider = ColliderComponent_get(components, n);
                    if (collider->last_collision == i) continue;

                    collider->last_collision = i;

                    sfVector2f ol = overlap(components, i, n);

                    if (ol.x == 0.0 && ol.y == 0.0) continue;

                    sfVector2f dv = physics->velocity;
                    sfVector2f no = normalized(ol);

                    float m = 1.0;

                    PhysicsComponent* other = PhysicsComponent_get(components, n);
                    if (other) {
                        dv = diff(dv, other->velocity);
                        m = other->mass / (physics->mass + other->mass);
                    }

                    sfVector2f new_vel = diff(physics->velocity, mult(2 * m * dot(dv, no), no));

                    switch (COLLISION_MATRIX[ColliderComponent_get(components, i)->group][collider->group]) {
                        case 1:
                            physics->collision.velocity = sum(physics->collision.velocity, new_vel);
                            physics->collision.overlap = sum(physics->collision.overlap, mult(m, ol));
                            physics->collision.collided = true;
                            break;
                        case 2:
                            apply_force(components, i, mult(50.0, ol));
                            break;
                        case 3:
                            VehicleComponent_get(components, i)->on_road = true;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
}


void draw_occupied_tiles(ComponentData* components, ColliderGrid* grid, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        if (!ColliderComponent_get(components, i)) continue;

        Bounds bounds = get_bounds(components, grid, i);
        for (int j = bounds.left; j <= bounds.right; j++) {
            for (int k = bounds.bottom; k <= bounds.top; k++) {
                sfVector2f pos = { j * grid->tile_width - 0.5 * grid->width + 0.5 * grid->tile_width, 
                                   k * grid->tile_height - 0.5 * grid->height + 0.5 * grid->tile_height };
                draw_rectangle(window, components, camera, NULL, pos, grid->tile_width, grid->tile_height, 0.0, get_color(1.0, 1.0, 1.0, 0.25));
            }
        }
    }
}


void debug_draw(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        ColliderComponent* col = components->collider[i];
        if (!col) continue;

        if (col->type == CIRCLE) {
            draw_circle(window, components, camera, NULL, get_position(components, i), col->radius, get_color(1.0, 0.0, 1.0, 0.25));
        } else {
            sfColor color = get_color(0.0, 1.0, 1.0, 0.25);
            draw_rectangle(window, components, camera, NULL, get_position(components, i), col->width, col->height, get_angle(components, i), color);
        }
    }
}
