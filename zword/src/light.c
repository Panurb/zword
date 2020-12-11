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


void draw_lights(ComponentData* component, ColliderGrid* grid, sfRenderWindow* window, TextureArray textures, sfRenderTexture* texture, Camera* camera, float ambient_light) {
    sfRenderTexture_clear(texture, sfColor_fromRGB(255 * ambient_light, 255 * ambient_light, 255 * ambient_light));

    for (int i = 0; i < component->entities; i++) {
        LightComponent* light = component->light[i];
        if (!light) continue;

        int j = texture_index("gradient");
        sfTexture_setRepeated(textures[j], sfFalse);
        sfConvexShape_setTexture(light->shape, textures[j], sfFalse);
        
        sfVector2f start = get_position(component, i);
        float angle = get_angle(component, i) - 0.5 * light->angle;

        sfRenderStates state = { sfBlendAdd, sfTransform_Identity, NULL, NULL };

        sfColor color = sfConvexShape_getFillColor(light->shape);
        color.a = light->brightness * 255;
        sfConvexShape_setFillColor(light->shape, color);

        sfVector2f velocity = polar_to_cartesian(1.0, angle);

        HitInfo info = raycast(component, grid, start, velocity, light->range, i);
        sfVector2f end = info.position;

        float delta_angle = light->angle / light->rays;

        sfVector2f r = polar_to_cartesian(camera->zoom * dist(start, end), -delta_angle);

        sfConvexShape_setPoint(light->shape, 0, (sfVector2f) { 0.0, 0.0 });
        sfConvexShape_setPoint(light->shape, 1, r);

        float old_dist = dist(start, end);

        for (int j = 1; j <= light->rays; j++) {
            angle += delta_angle;
            velocity = polar_to_cartesian(1.0, angle);

            info = raycast(component, grid, start, velocity, light->range, i);
            end = info.position;

            end = sum(end, mult(0.25, velocity));

            r = polar_to_cartesian(camera->zoom * dist(start, end), 0.0);

            sfConvexShape_setPoint(light->shape, 2, r);

            sfConvexShape_setPosition(light->shape, world_to_texture(start, camera));
            sfConvexShape_setRotation(light->shape, to_degrees(angle));
            int width = 128 * dist(start, end) / light->range;
            if (dist(start, end) < old_dist) {
                width = 128 * old_dist / light->range;
            }
            sfConvexShape_setTextureRect(light->shape, (sfIntRect) { 0, 0, width, 128 });

            sfRenderTexture_drawConvexShape(texture, light->shape, &state);

            old_dist = dist(start, end);

            sfConvexShape_setPoint(light->shape, 1, polar_to_cartesian(camera->zoom * old_dist, -delta_angle));

            ImageComponent* image = ImageComponent_get(component, info.object);
            if (!image) continue;

            if (image->shine > 0.0) {
                float vn = dot(velocity, info.normal);
                if (vn < -0.98) {
                    sfCircleShape_setPosition(light->shine, world_to_screen(sum(end, mult(0.05, info.normal)), camera));

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
