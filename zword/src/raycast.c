#define _USE_MATH_DEFINES

#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "component.h"
#include "util.h"
#include "collider.h"
#include "raycast.h"
#include "game.h"


typedef struct {
    float time;
    Vector2f normal;
} Hit;


Hit ray_intersection(int i, Vector2f start, Vector2f velocity, float range) {
    Hit hit = { range, perp(velocity) };

    ColliderComponent* col = ColliderComponent_get(i);

    if (!col->enabled) return hit;

    if (col->type == COLLIDER_RECTANGLE) {
        // https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282

        Vector2f corners[4];
        get_corners(i, corners);

        int n = 0;
        for (int k = 0; k < 4; k++) {
            Vector2f dir = diff(corners[(k + 1) % 4], corners[k]);
            float t = cross(diff(corners[k], start), dir) / cross(velocity, dir);
            float u = cross(diff(start, corners[k]), velocity) / cross(dir, velocity);

            // Aiming ray directly at corner can sometimes slip through so add some tolerance
            float delta = 0.001f;
            if (t >= 0.0 && u >= -delta && u <= 1.0f + delta) {
                n++;
                if (t < hit.time) {
                    hit.time = t;
                    hit.normal = perp(dir);
                }
            }
        }
    } else {
        // https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

        float radius = col->radius;
        Vector2f oc = diff(start, get_position(i));
        float delta = powf(dot(velocity, oc), 2) - norm2(oc) + powf(radius, 2);
        float t = -dot(velocity, oc) - sqrtf(delta);

        if (delta >= 0.0 && t >= 0.0 && t < hit.time) {
            hit.time = t;
            Vector2f p = sum(start, mult(t, velocity));
            hit.normal = diff(p, get_position(i));
        }
    }

    return hit;
}


HitInfo raycast(Vector2f start, Vector2f velocity, float range, ColliderGroup group) {
    // http://www.cs.yorku.ca/~amana/research/grid.pdf

    static int id = MAX_ENTITIES;
    id = (id < 2 * MAX_ENTITIES) ? id + 1 : MAX_ENTITIES;

    float v = norm(velocity);
    velocity = mult(1.0f / v, velocity);

    HitInfo info;
    info.entity = -1;
    info.normal = perp(velocity);
    info.position = zeros();
    info.distance = range;

    if (v == 0.0f) {
        LOG_WARNING("Raycast with zero velocity");
        return info;
    }

    ColliderGrid* grid = game_data->grid;

    int x = floorf((start.x + 0.5 * grid->width) / grid->tile_width);
    int y = floorf((start.y + 0.5 * grid->height) / grid->tile_height);

    int step_x = sign(velocity.x);
    int step_y = sign(velocity.y);

    float t_max_x = (grid->tile_width - mod(step_x * (start.x + 1e-6), grid->tile_width)) / fabs(velocity.x);
    float t_max_y = (grid->tile_height - mod(step_y * (start.y + 1e-6), grid->tile_height)) / fabs(velocity.y);

    float t_delta_x = grid->tile_width / fabs(velocity.x);
    float t_delta_y = grid->tile_height / fabs(velocity.y);
    
    float t_min = range;

    while (x > 0 && x < grid->columns && y > 0 && y < grid->rows) {
        for (ListNode* current = grid->array[x][y]->head; current != NULL; current = current->next) {
            int j = current->value;
            if (j == -1) continue;

            ColliderComponent* col = ColliderComponent_get(j);
            if (col->last_collision == id) continue;
            if (COLLISION_MATRIX[group][col->group] == 0) continue;
            if (col->trigger_type != TRIGGER_NONE) continue;

            col->last_collision = id;

            // Skip if ray starts inside collider
            if (point_inside_collider(j, start)) {
                continue;
            }

            Hit hit = ray_intersection(j, start, velocity, range);
            if (hit.time < t_min) {
                t_min = hit.time;
                info.entity = j;
                info.normal = hit.normal;
                info.distance = t_min;
                range = t_min;
            }
        }

        if (fminf(t_max_x, t_max_y) > range) break;

        if (t_max_x < t_max_y) {
            t_max_x += t_delta_x;
            x += step_x;
        } else {
            t_max_y += t_delta_y;
            y += step_y;
        }
    }

    info.position = sum(start, mult(t_min, velocity));
    info.normal = normalized(info.normal);

    return info;
}
