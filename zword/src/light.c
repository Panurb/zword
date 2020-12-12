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


void update_lights(ComponentData* component, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        LightComponent* light = component->light[i];

        if (!light) continue;

        if (component->coordinate[i]->parent != -1) {
            VehicleComponent* vehicle = component->vehicle[component->coordinate[i]->parent];
            if (vehicle) {
                light->enabled = (vehicle->riders[0] != -1);
            }
        }

        if (light->enabled) {
            light->brightness = fmin(light->max_brightness, light->brightness + light->speed * delta_time);
        } else {
            light->brightness = fmax(0.0, light->brightness - light->speed * delta_time);
        }
    }
}


void draw_shine(ComponentData* components, sfRenderWindow* window, Camera* camera, int entity, HitInfo info, sfVector2f velocity) {
    ImageComponent* image = ImageComponent_get(components, info.object);
    if (!image) return;

    LightComponent* light = components->light[entity];

    if (image->shine > 0.0) {
        float vn = dot(velocity, info.normal);
        if (vn < -0.98) {
            sfCircleShape_setPosition(light->shine, world_to_screen(sum(info.position, mult(0.05, info.normal)), camera));

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


void draw_lights(ComponentData* component, ColliderGrid* grid, sfRenderWindow* window, TextureArray textures, sfRenderTexture* texture, Camera* camera, float ambient_light) {
    sfRenderTexture_clear(texture, get_color(ambient_light, ambient_light, ambient_light, 1.0));
    sfRenderStates state = { sfBlendAdd, sfTransform_Identity, NULL, NULL };

    for (int i = 0; i < component->entities; i++) {
        LightComponent* light = component->light[i];
        if (!light) continue;

        sfVector2f start = get_position(component, i);
        float angle = get_angle(component, i) - 0.5 * light->angle;

        sfVertex* v = sfVertexArray_getVertex(light->verts, 0);
        v->position = world_to_texture(start, camera);
        sfColor color = light->color;
        color.a = 255 * light->brightness;
        v->color = color;

        sfVector2f velocity = polar_to_cartesian(1.0, angle);

        HitInfo info = raycast(component, grid, start, velocity, light->range, i);
        sfVector2f end = info.position;

        float delta_angle = light->angle / (light->rays - 1);
        Matrix2f rot = rotation_matrix(delta_angle);

        for (int j = 1; j < light->rays + 1; j++) {
            velocity = matrix_mult(rot, velocity);

            info = raycast(component, grid, start, velocity, light->range, i);
            end = info.position;

            end = sum(end, mult(-0.25, info.normal));

            v = sfVertexArray_getVertex(light->verts, j);
            v->position = world_to_texture(end, camera);
            color.a = 255 * light->brightness * (1.0 - dist(start, end) / (light->range + 0.25));
            v->color = color;
        }

        sfRenderTexture_drawVertexArray(texture, light->verts, &state);
    }
}
