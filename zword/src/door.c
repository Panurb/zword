#define _USE_MATH_DEFINES
#include <math.h>

#include "component.h"
#include "util.h"
#include "grid.h"


void update_doors(ComponentData* components, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        DoorComponent* door = DoorComponent_get(components, i);
        if (!door) continue;

        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        ColliderComponent* col = ColliderComponent_get(components, i);

        if (!col) continue;

        if (door->locked) {
            col->group = GROUP_WALLS;
            coord->position = sum(door->anchor, door->direction);
        } else {
            col->group = GROUP_DOORS;

            sfVector2f r = diff(get_position(components, i), door->anchor);
            coord->angle = polar_angle(r);

            float angle = signed_angle(door->direction, r);
            if (fabsf(angle) > 0.5f * M_PI) {
                coord->angle = polar_angle(door->direction) + sign(angle) * 0.5f * M_PI;
            }

            clear_grid(components, grid, i);
            coord->position = sum(door->anchor, polar_to_cartesian(1.0f, coord->angle));
            update_grid(components, grid, i);
        }
    }
}
