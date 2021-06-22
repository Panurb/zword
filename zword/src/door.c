#define _USE_MATH_DEFINES
#include <math.h>

#include "component.h"
#include "util.h"
#include "grid.h"
#include "sound.h"


void create_door(ComponentData* components, sfVector2f pos, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, angle);
    ImageComponent_add(components, i, "door", 2.0f, 1.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 2.0f, 0.3f, GROUP_DOORS);
    ParticleComponent_add_splinter(components, i);
    // HealthComponent_add(components, i, 200, "", "", "wood_destroy");
    SoundComponent_add(components, i, "wood_hit");
    DoorComponent_add(components, i, sum(pos, polar_to_cartesian(-1.0f, angle)));
    PhysicsComponent_add(components, i, 1.0f);
}


void update_doors(ComponentData* components, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        DoorComponent* door = DoorComponent_get(components, i);
        if (!door) continue;

        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        ColliderComponent* col = ColliderComponent_get(components, i);
        PhysicsComponent* phys = PhysicsComponent_get(components, i);

        if (!col) continue;

        if (door->locked) {
            col->group = GROUP_WALLS;
            coord->position = sum(door->anchor, door->direction);
        } else {
            col->group = GROUP_DOORS;

            float old_angle = coord->angle;
            sfVector2f r = diff(get_position(components, i), door->anchor);
            coord->angle = polar_angle(r);

            float angle = signed_angle(door->direction, r);
            if (fabsf(angle) > 0.5f * M_PI) {
                coord->angle = mod(polar_angle(door->direction) + sign(angle) * 0.5f * M_PI, 2.0f * M_PI);
            }

            clear_grid(components, grid, i);
            coord->position = sum(door->anchor, polar_to_cartesian(1.0f, coord->angle));
            update_grid(components, grid, i);

            // float delta_angle = fabsf(mod(fabsf(old_angle - coord->angle), 2.0f * M_PI));
            // printf("%f\n", coord->angle);
            // if (delta_angle > 3.0f) {
            //     add_sound(components, i, "door", 0.5f, randf(0.9f, 1.1f));
            // }
            if (phys->speed > 0.0f) {
                loop_sound(components, i, "door", 0.5f, 1.0f);
            } else {
                stop_loop(components, i);
            }
        }
    }
}
