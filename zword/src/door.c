#define _USE_MATH_DEFINES
#include <math.h>

#include "component.h"
#include "util.h"
#include "grid.h"
#include "sound.h"
#include "particle.h"


void create_door(ComponentData* components, sfVector2f pos, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, angle);
    ImageComponent_add(components, i, "door", 2.0f, 1.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 2.0f, 0.3f, GROUP_DOORS);
    ParticleComponent_add_splinter(components, i);
    SoundComponent_add(components, i, "wood_hit");
    DoorComponent_add(components, i, sum(pos, polar_to_cartesian(-1.0f, angle)));
    PhysicsComponent_add(components, i, 1.0f);
    int j = create_entity(components);
    CoordinateComponent_add(components, j, sum(pos, polar_to_cartesian(1.0f, angle + M_PI)), angle + M_PI);
    JointComponent_add(components, i, j, 1.0f, 1.0f, INFINITY)->max_angle = 0.5f * M_PI;
}


void update_doors(ComponentData* components, ColliderGrid* grid) {
    return;
    for (int i = 0; i < components->entities; i++) {
        DoorComponent* door = DoorComponent_get(components, i);
        if (!door) continue;

        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        ColliderComponent* col = ColliderComponent_get(components, i);
        PhysicsComponent* phys = PhysicsComponent_get(components, i);

        if (!col) continue;

        if (door->locked) {
            col->group = GROUP_WALLS;
        } else {
            col->group = GROUP_DOORS;

            if (phys->speed > 0.25f) {
                loop_sound(components, i, "door", 0.5f, 1.0f);
            } else {
                stop_loop(components, i);
            }
        }
    }
}
