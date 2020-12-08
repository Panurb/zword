#include <math.h>

#include "component.h"
#include "util.h"
#include "collider.h"
#include "raycast.h"


float ray_intersection(Component* component, int i, sfVector2f start, sfVector2f velocity, float range) {
    float t_min = range;

    CoordinateComponent* coord = component->coordinate[i];
    ColliderComponent* col = component->collider[i];

    if (!col->enabled) return t_min;

    if (col->type == RECTANGLE) {
        //https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282

        sfVector2f hw = half_width(component, i);
        sfVector2f hh = half_height(component, i);
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
                t_min = t;
            }
        }
    } else {
        //https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

        float radius = col->radius;
        sfVector2f oc = diff(start, coord->position);
        float delta = powf(dot(velocity, oc), 2) - norm2(oc) + powf(radius, 2);
        float t = -dot(velocity, oc) - sqrtf(delta);

        if (delta >= 0.0 && t >= 0.0 && t < t_min) {
            t_min = t;
        }
    }

    return t_min;
}


void calculate_normal(Component* component, HitInfo info) {
    ColliderComponent* col = component->collider[info.object];
    
    sfVector2f r = normalized(diff(info.position, component->coordinate[info.object]->position));
    if (col->type == CIRCLE) {
        info.normal = r;
    } else {
        sfVector2f hw = polar_to_cartesian(1.0, component->coordinate[info.object]->angle);
        sfVector2f hh = polar_to_cartesian(1.0, component->coordinate[info.object]->angle + 0.5 * M_PI);

        float width = col->width;
        float height = col->height;

        float rhw = dot(r, hw);
        float rhh = dot(r, hh);
        if (0.5 * width - fabs(rhw) > 0.5 * height - fabs(rhh)) {
            info.normal = mult(sign(rhh), hh);
        } else {
            info.normal = mult(sign(rhw), hw);
        }
    }
}


HitInfo raycast(Component* component, ColliderGrid* grid, sfVector2f start, sfVector2f velocity, float range, int ignore) {
    //http://www.cs.yorku.ca/~amana/research/grid.pdf

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

    int objects[20];
    for (int i = 0; i < 20; i++) {
        objects[i] = -1;
    }

    while (x > 0 && x < grid->width && y > 0 && y < grid->height) {
        for (int i = 0; i < grid->size; i++) {
            int j = grid->array[x][y][i];
            if (j == -1) continue;
            if (j == ignore) continue;

            bool found = false;
            for (int k = 0; k < 20; k++) {
                if (objects[k] == -1) {
                    objects[k] = j;
                    break;
                } else if (objects[k] == j) {
                    found = true;
                    break;
                }
            }
            if (found) continue;

            float t = ray_intersection(component, j, start, velocity, range);
            if (t < t_min) {
                t_min = t;
                info.object = j;
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
    if (info.object != -1) {
        calculate_normal(component, info);
    }

    return info;
}
