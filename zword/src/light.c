#include <math.h>

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "collider.h"
#include "util.h"
#include "light.h"
#include "grid.h"


void draw_light(Component* component, ColliderGrid* grid, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->light[i]) continue;

        LightComponent* light = component->light[i];
        CoordinateComponent* coord = component->coordinate[i];

        float angle = coord->angle;

        sfVector2f velocity = polar_to_cartesian(0.6, angle - 0.5 * light->angle);

        sfVector2f points[4];
        points[0] = sum(coord->position, velocity);
        points[1] = raycast(component, grid, points[0], velocity, light->range);

        for (int j = 1; j <= light->rays; j++) {
            velocity = polar_to_cartesian(0.6, angle - 0.5 * light->angle + j * (light->angle / light->rays));
            points[3] = sum(coord->position, velocity);
            points[2] = raycast(component, grid, points[3], velocity, light->range);
            points[2] = sum(points[2], mult(0.5, velocity));

            sfConvexShape* shape = sfConvexShape_create();
            sfConvexShape_setPointCount(shape, 4);
            for (int k = 0; k < 4; k++) {
                sfConvexShape_setPoint(shape, k, world_to_screen(points[k], camera));
            }

            sfConvexShape_setFillColor(shape, sfColor_fromRGBA(255, 255, 255, light->brightness * 255));
            sfRenderWindow_drawConvexShape(window, shape, NULL);
            sfConvexShape_destroy(shape);

            points[0] = points[3];
            points[1] = points[2];
        }
    }
}


float ray_intersection(Component* component, int i, sfVector2f start, sfVector2f velocity, float range) {
    float t_min = range;

    CoordinateComponent* coord = component->coordinate[i];
    if (component->rectangle_collider[i]) {
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
    } else if (component->circle_collider[i]) {
        //https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection

        float radius = component->circle_collider[i]->radius;
        sfVector2f oc = diff(start, coord->position);
        float delta = powf(dot(velocity, oc), 2) - norm2(oc) + powf(radius, 2);
        float t = -dot(velocity, oc) - sqrtf(delta);

        if (delta >= 0.0 && t >= 0.0 && t < t_min) {
            t_min = t;
        }
    }

    return t_min;
}


sfVector2f raycast(Component* component, ColliderGrid* grid, sfVector2f start, sfVector2f velocity, float range) {
    //http://www.cs.yorku.ca/~amana/research/grid.pdf

    velocity = normalized(velocity);

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

    return sum(start, mult(t_min, velocity));
}
