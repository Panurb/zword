#define _USE_MATH_DEFINES

#include <math.h>
#include <stdlib.h>
#include <limits.h>

#include "component.h"
#include "util.h"
#include "collider.h"
#include "raycast.h"


typedef struct {
    float time;
    sfVector2f normal;
} Hit;


Hit ray_intersection(ComponentData* component, int i, sfVector2f start, sfVector2f velocity, float range) {
    Hit hit = { range, perp(velocity) };

    ColliderComponent* col = component->collider[i];

    if (!col->enabled) return hit;

    if (col->type == RECTANGLE) {
        // https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282

        sfVector2f corners[4];
        get_corners(component, i, corners);

        for (int k = 0; k < 4; k++) {
            sfVector2f dir = diff(corners[(k + 1) % 4], corners[k]);
            float t = cross(diff(corners[k], start), dir) / cross(velocity, dir);
            float u = cross(diff(start, corners[k]), velocity) / cross(dir, velocity);

            if (t >= 0.0 && t < hit.time && u >= 0.0 && u <= 1.0) {
                hit.time = t;
                hit.normal = perp(dir);
            }
        }
    } else {
        // https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

        float radius = col->radius;
        sfVector2f oc = diff(start, get_position(component, i));
        float delta = powf(dot(velocity, oc), 2) - norm2(oc) + powf(radius, 2);
        float t = -dot(velocity, oc) - sqrtf(delta);

        if (delta >= 0.0 && t >= 0.0 && t < hit.time) {
            hit.time = t;
            sfVector2f p = sum(start, mult(t, velocity));
            hit.normal = diff(p, get_position(component, i));
        }
    }

    return hit;
}


HitInfo raycast(ComponentData* component, ColliderGrid* grid, sfVector2f start, sfVector2f velocity, float range, int ignore) {
    // http://www.cs.yorku.ca/~amana/research/grid.pdf

    static int id = MAX_ENTITIES;

    id = (id < INT_MAX) ? id + 1 : MAX_ENTITIES;

    velocity = normalized(velocity);

    HitInfo info;
    info.object = -1;
    info.normal = perp(velocity);

    int x = floor(start.x / grid->tile_width + 0.5 * grid->width);
    int y = floor(start.y / grid->tile_height + 0.5 * grid->height);

    int step_x = sign(velocity.x);
    int step_y = sign(velocity.y);

    float t_max_x = (grid->tile_width - mod(step_x * start.x, grid->tile_width)) / fabs(velocity.x);
    float t_max_y = (grid->tile_height - mod(step_y * start.y, grid->tile_height)) / fabs(velocity.y);

    float t_delta_x = grid->tile_width / fabs(velocity.x);
    float t_delta_y = grid->tile_height / fabs(velocity.y);
    
    float t_min = range;

    while (x > 0 && x < grid->width && y > 0 && y < grid->height) {
        for (int i = 0; i < grid->size; i++) {
            int j = grid->array[x][y][i];
            if (j == -1) continue;
            if (j == ignore) continue;
            if (component->collider[j]->last_collision == id) continue;

            component->collider[j]->last_collision = id;

            Hit hit = ray_intersection(component, j, start, velocity, range);
            if (hit.time < t_min) {
                t_min = hit.time;
                info.object = j;
                info.normal = hit.normal;

                // not sure if correct
                range = t_min;
            }
        }

        if (t_max_x < t_max_y) {
            t_max_x += t_delta_x;
            x += step_x;
        } else {
            t_max_y += t_delta_y;
            y += step_y;
        }

        if (min(t_max_x, t_max_y) > range) break;
    }

    info.position = sum(start, mult(t_min, velocity));
    info.normal = normalized(info.normal);

    return info;
}