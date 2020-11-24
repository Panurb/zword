#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "collider.h"
#include "util.h"
#include "light.h"


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
