#define _USE_MATH_DEFINES

#include <math.h>

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "collider.h"
#include "util.h"
#include "light.h"
#include "grid.h"
#include "raycast.h"
#include "image.h"


void update_lights(ComponentData* components, float delta_time) {
    for (int i = 0; i < components->entities; i++) {
        LightComponent* light = components->light[i];

        if (!light) continue;

        if (components->coordinate[i]->parent != -1) {
            VehicleComponent* vehicle = components->vehicle[components->coordinate[i]->parent];
            if (vehicle) {
                light->enabled = (vehicle->riders[0] != -1);
            }
        }

        if (light->enabled) {
            light->brightness = fminf(light->max_brightness, light->brightness + light->speed * delta_time);
        } else {
            light->brightness = fmaxf(0.0, light->brightness - light->speed * delta_time);
        }

        light->time = fmodf(light->time + delta_time, 400.0f * M_PI);
    }
}


void draw_shine(ComponentData* components, sfRenderWindow* window, int camera, int entity, HitInfo info, sfVector2f velocity) {
    ImageComponent* image = ImageComponent_get(info.entity);
    if (!image) return;

    LightComponent* light = components->light[entity];

    if (image->shine > 0.0) {
        float vn = dot(velocity, info.normal);
        if (vn < -0.98) {
            sfCircleShape_setPosition(light->shine, world_to_screen(components, camera, sum(info.position, mult(0.05, info.normal))));

            sfColor color = sfWhite;
            color.a = (1 - 50 * (1 + vn)) * 128;
            sfCircleShape_setFillColor(light->shine, color);
            float radius = 0.05 * (1 - 5 * (1 + vn)) * CameraComponent_get(camera)->zoom;
            sfCircleShape_setRadius(light->shine, radius);
            sfCircleShape_setOrigin(light->shine, (sfVector2f) { radius, radius });
            sfRenderWindow_drawCircleShape(window, light->shine, NULL);
        }
    }
}


void draw_shadows(ComponentData* components, sfRenderTexture* texture, int camera) {
    sfRenderTexture_clear(texture, sfWhite);
    if (CameraComponent_get(camera)->zoom < 10.0f) return;

    sfRenderStates state = { sfBlendAlpha, sfTransform_Identity, NULL, NULL };
    for (int i = 0; i < components->entities; i++) {
        ColliderComponent* collider = ColliderComponent_get(i);
        if (!collider) continue;

        ImageComponent* image = ImageComponent_get(i);
        if (!image || image->alpha == 0.0f || image->layer <= LAYER_DECALS) continue;

        sfVector2f start = get_position(i);

        float radius = 1.5f * collider->radius;
        if (!on_screen(components, camera, start, 2.0f * radius, 2.0f * radius)) {
            continue;
        }
        
        switch (collider->type) {
            case COLLIDER_CIRCLE: {
                sfVertex* v = sfVertexArray_getVertex(collider->verts, 0);
                v->position = world_to_texture(components, camera, start);
                sfColor color = sfBlack;
                color.a = 128;
                v->color = color;

                sfVector2f velocity = polar_to_cartesian(radius, 0.0f);

                float delta_angle = 2.0f * M_PI / (collider->verts_size - 2);
                Matrix2f rot = rotation_matrix(delta_angle);

                color.a = 0;
                for (int j = 1; j < collider->verts_size; j++) {
                    sfVector2f end = sum(start, velocity);

                    v = sfVertexArray_getVertex(collider->verts, j);
                    v->position = world_to_texture(components, camera, end);
                    v->color = color;

                    velocity = matrix_mult(rot, velocity);
                }
                break;
            } case COLLIDER_RECTANGLE: {
                sfVector2f corners[8];
                get_corners(components, i, corners);

                float r = fminf(0.5f * radius, 1.0f);
                float angle = get_angle(i);
                for (int j = 0; j < 4; j++) {
                    sfVector2f corner = corners[j];
                    corners[j + 4] = sum(corner, polar_to_cartesian(r, angle + (0.25f - j * 0.5f) * M_PI));
                }

                // 7 ----------- 4
                // |\           /|
                // |  3 ----- 0  |
                // |  |       |  |
                // |  2 ----- 1  |
                // |/           \|
                // 6 ----------- 5

                sfColor color = sfBlack;
                int quads[20] = { 2, 3, 7, 6,
                                  3, 0, 4, 7,
                                  0, 1, 5, 4,
                                  1, 2, 6, 5,
                                  3, 2, 1, 0 };
                for (int j = 0; j < collider->verts_size; j++) {
                    sfVertex* v = sfVertexArray_getVertex(collider->verts, j);
                    v->position = world_to_texture(components, camera, corners[quads[j]]);
                    color.a = j % 4 < 2 || j > 15 ? 64 : 0;
                    v->color = color;
                }

                break;
            }
        }

        sfRenderTexture_drawVertexArray(texture, collider->verts, &state);
    }
}


void draw_lights(ComponentData* components, ColliderGrid* grid, sfRenderTexture* texture, int camera, float ambient_light) {
    sfRenderTexture_clear(texture, get_color(ambient_light, ambient_light, ambient_light, 1.0f));
    if (CameraComponent_get(camera)->zoom < 10.0f) return;

    sfRenderStates state = { sfBlendAdd, sfTransform_Identity, NULL, NULL };
    for (int i = 0; i < components->entities; i++) {
        LightComponent* light = LightComponent_get(i);
        if (!light) continue;

        sfVector2f start = get_position(i);

        if (!on_screen(components, camera, start, 2.0f * light->range, 2.0f * light->range)) {
            continue;
        }

        float brightness = light->brightness;
        float range = light->range;

        if (light->enabled) {
            float f = 1.0 - light->flicker * 0.25 * (sinf(8.0 * light->time) + sinf(12.345 * light->time) + 2.0);
            brightness *= f;
            range *= f;
        }

        sfVertex* v = sfVertexArray_getVertex(light->verts, 0);
        v->position = world_to_texture(components, camera, start);
        sfColor color = light->color;
        color.a = 255 * brightness;
        v->color = color;

        float angle = get_angle(i) - 0.5 * light->angle;
        sfVector2f velocity = polar_to_cartesian(1.0, angle);

        float delta_angle = light->angle / (light->rays - 1);
        Matrix2f rot = rotation_matrix(delta_angle);

        for (int j = 1; j < light->rays + 1; j++) {
            HitInfo info = raycast(components, grid, start, velocity, range, GROUP_LIGHTS);
            sfVector2f end = info.position;

            end = sum(end, mult(0.25, velocity));

            v = sfVertexArray_getVertex(light->verts, j);
            v->position = world_to_texture(components, camera, end);
            color.a = 255 * brightness * (1.0 - dist(start, end) / (range + 0.25));
            v->color = color;

            velocity = matrix_mult(rot, velocity);
        }

        sfRenderTexture_drawVertexArray(texture, light->verts, &state);
    }
}
