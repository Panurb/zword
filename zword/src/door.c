#define _USE_MATH_DEFINES
#include <math.h>

#include "component.h"
#include "util.h"
#include "grid.h"
#include "sound.h"
#include "particle.h"
#include "navigation.h"


int create_door(ComponentData* components, sfVector2f pos, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(i, pos, angle + M_PI);
    ImageComponent_add(i, "door", 2.0f, 1.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(components, i, 2.0f, 0.3f, GROUP_DOORS);
    ParticleComponent_add_type(components, i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(i, "wood_hit");
    DoorComponent_add(i, 500);
    PhysicsComponent_add(i, 1.0f);

    int j = create_entity(components);
    CoordinateComponent_add(j, sum(pos, polar_to_cartesian(1.0f, angle + M_PI)), angle + M_PI);
    ImageComponent_add(j, "hinge", 0.0f, 0.0f, LAYER_ITEMS);
    JointComponent_add(i, j, 1.0f, 1.0f, 1.0f)->max_angle = 0.5f * M_PI;

    return i;
}


void update_doors(ComponentData* components) {
    for (int i = 0; i < components->entities; i++) {
        DoorComponent* door = DoorComponent_get(i);
        if (!door) continue;

        ColliderComponent* col = ColliderComponent_get(i);
        PhysicsComponent* phys = PhysicsComponent_get(i);

        if (!col) continue;

        if (door->locked) {
            col->group = GROUP_WALLS;
        } else {
            col->group = GROUP_DOORS;

            if (phys->speed > 0.25f) {
                loop_sound(components, i, "door", 0.3f, 1.0f);
            } else {
                stop_loop(components, i);
            }
        }
    }
}
