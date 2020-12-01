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


void update_lights(Component* component, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        LightComponent* light = component->light[i];

        if (!light) continue;

        if (component->coordinate[i]->parent != -1) {
            VehicleComponent* vehicle = component->vehicle[component->coordinate[i]->parent];
            if (vehicle) {
                light->enabled = (vehicle->driver != -1);
            }
        }

        if (light->enabled) {
            light->brightness = fmin(light->max_brightness, light->brightness + light->speed * delta_time);
        } else {
            light->brightness = fmax(0.0, light->brightness - light->speed * delta_time);
        }
    }
}


void draw_lights(Component* component, ColliderGrid* grid, sfRenderWindow* window, sfRenderTexture* texture, Camera* camera, float ambient_light) {
    sfRenderTexture_clear(texture, sfColor_fromRGB(255 * ambient_light, 255 * ambient_light, 255 * ambient_light));

    for (int i = 0; i < component->entities; i++) {
        if (!component->light[i]) continue;

        LightComponent* light = component->light[i];
        sfVector2f position = get_position(component, i);
        float angle = get_angle(component, i);

        sfRenderStates state = { sfBlendAdd, sfTransform_Identity, NULL, NULL };

        float brightness = light->brightness;
        if (light->smoothing != 0) {
            brightness *= 1.0 / light->smoothing;
        }
        
        sfColor color = sfConvexShape_getFillColor(light->shape);
        color.a = brightness * 255;
        sfConvexShape_setFillColor(light->shape, color);

        for (int k = 0; k <= light->smoothing; k++) {
            float range = light->range - 0.2 * fabs(k - light->smoothing / 2);

            sfVector2f velocity = polar_to_cartesian(1.0, angle - 0.5 * light->angle);
            sfVector2f start = sum(position, polar_to_cartesian((k - light->smoothing / 2) * 0.1, angle + 0.5 * M_PI));

            sfConvexShape_setPoint(light->shape, 0, world_to_texture(start, camera));

            sfVector2f end = raycast(component, grid, start, velocity, range).position;
            sfConvexShape_setPoint(light->shape, 1, world_to_texture(end, camera));

            for (int j = 1; j <= light->rays; j++) {
                velocity = polar_to_cartesian(1.0, angle - 0.5 * light->angle + j * (light->angle / light->rays));

                HitInfo info = raycast(component, grid, start, velocity, range);
                sfVector2f end = info.position;

                sfVector2f offset = mult(0.25 - 0.05 * fabs(k - light->smoothing / 2), velocity);

                sfConvexShape_setPoint(light->shape, j % 2 + 1, world_to_texture(sum(end, offset), camera));

                sfRenderTexture_drawConvexShape(texture, light->shape, &state);

                if (component->image[i] && component->image[i]->shine > 0.0) {
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
}
