#define _USE_MATH_DEFINES

#include <math.h>

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "collider.h"
#include "util.h"
#include "light.h"
#include "grid.h"
#include "light.h"


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


HitInfo raycast(Component* component, ColliderGrid* grid, sfVector2f start, sfVector2f velocity, float range) {
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
        sfVector2f r = normalized(diff(info.position, component->coordinate[info.object]->position));
        if (component->circle_collider[info.object]) {
            info.normal = r;
        } else if (component->rectangle_collider[info.object]) {
            sfVector2f hw = polar_to_cartesian(1.0, component->coordinate[info.object]->angle);
            sfVector2f hh = polar_to_cartesian(1.0, component->coordinate[info.object]->angle + 0.5 * M_PI);

            float width = component->rectangle_collider[info.object]->width;
            float height = component->rectangle_collider[info.object]->height;

            float rhw = dot(r, hw);
            float rhh = dot(r, hh);
            if (0.5 * width - fabs(rhw) > 0.5 * height - fabs(rhh)) {
                info.normal = mult(sign(rhh), hh);
            } else {
                info.normal = mult(sign(rhw), hw);
            }
        }
    }

    return info;
}


void draw_lights(Component* component, ColliderGrid* grid, sfRenderWindow* window, sfRenderTexture* texture, Camera* camera, float ambient_light) {
    sfRenderTexture_clear(texture, sfColor_fromRGB(255 * ambient_light, 255 * ambient_light, 255 * ambient_light));

    for (int i = 0; i < component->entities; i++) {
        if (!component->light[i]) continue;

        LightComponent* light = component->light[i];
        CoordinateComponent* coord = component->coordinate[i];

        sfRenderStates state = { sfBlendAdd, sfTransform_Identity, NULL, NULL };

        for (int k = 0; k <= light->smoothing; k++) {
            float range = light->range - 0.2 * fabs(k - light->smoothing / 2);

            sfVector2f velocity = polar_to_cartesian(1.0, coord->angle - 0.5 * light->angle);
            sfVector2f start = sum(coord->position, polar_to_cartesian((k - light->smoothing / 2) * 0.1, coord->angle + 0.5 * M_PI));

            sfConvexShape_setPoint(light->shape, 0, world_to_texture(start, camera));

            sfVector2f end = raycast(component, grid, start, velocity, range).position;
            sfConvexShape_setPoint(light->shape, 1, world_to_texture(end, camera));

            for (int j = 1; j <= light->rays; j++) {
                velocity = polar_to_cartesian(1.0, coord->angle - 0.5 * light->angle + j * (light->angle / light->rays));

                HitInfo info = raycast(component, grid, start, velocity, range);
                sfVector2f end = info.position;

                sfVector2f offset = mult(0.25 - 0.05 * fabs(k - light->smoothing / 2), velocity);

                sfConvexShape_setPoint(light->shape, j % 2 + 1, world_to_texture(sum(end, offset), camera));

                //float brightness = light->brightness * 4 / pow(light->rays, 2) * (j * (-j + light->rays));
                float brightness = light->brightness;
                if (light->smoothing != 0) {
                    brightness *= 1.0 / light->smoothing;
                }
                
                sfColor color = light->color;
                color.a = brightness * 255;
                sfConvexShape_setFillColor(light->shape, color);

                sfRenderTexture_drawConvexShape(texture, light->shape, &state);

                sfCircleShape_setPosition(light->shine, world_to_screen(diff(end, mult(0.14, info.normal)), camera));

                float vn = dot(velocity, info.normal);
                if (vn < -0.98) {
                    sfColor color = sfWhite;
                    color.a = (1 - 50 * (1 + vn)) * 128;
                    sfCircleShape_setFillColor(light->shine, color);
                    float radius = 0.05 * (1 - 5 * (1 + vn)) * camera->zoom;
                    sfCircleShape_setRadius(light->shine, radius);
                    sfCircleShape_setOrigin(light->shine, (sfVector2f) { radius, radius });
                    sfRenderWindow_drawCircleShape(window, light->shine, NULL);
                }
            }
        }
    }
}
