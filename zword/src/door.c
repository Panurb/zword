#define _USE_MATH_DEFINES
#include <math.h>

#include "component.h"
#include "util.h"
#include "grid.h"
#include "sound.h"
#include "particle.h"
#include "navigation.h"
#include "game.h"
#include "player.h"


int create_door(Vector2f pos, float angle) {
    int i = create_entity();
    CoordinateComponent_add(i, pos, angle + M_PI);
    ImageComponent_add(i, "door", 2.0f, 1.0f, LAYER_ITEMS);
    ColliderComponent_add_rectangle(i, 2.0f, 0.3f, GROUP_DOORS);
    ParticleComponent_add_type(i, PARTICLE_SPLINTER, 0.0f);
    SoundComponent_add(i, "wood_hit");
    DoorComponent_add(i, 500);
    PhysicsComponent_add(i, 1.0f);

    int j = create_entity();
    CoordinateComponent_add(j, sum(pos, polar_to_cartesian(1.0f, angle + M_PI)), angle + M_PI);
    ImageComponent_add(j, "hinge", 0.0f, 0.0f, LAYER_ITEMS);
    JointComponent_add(i, j, 1.0f, 1.0f, 1.0f)->max_angle = 0.5f * M_PI;

    return i;
}


void update_doors() {
    for (int i = 0; i < game_data->components->entities; i++) {
        DoorComponent* door = DoorComponent_get(i);
        if (!door) continue;

        ColliderComponent* col = ColliderComponent_get(i);
        PhysicsComponent* phys = PhysicsComponent_get(i);

        if (!col) continue;

        if (door->locked) {
            col->group = GROUP_WALLS;
            phys->lock = AXIS_ALL;
        } else {
            col->group = GROUP_DOORS;
            phys->lock = AXIS_NONE;

            if (phys->speed > 0.25f) {
                loop_sound(i, "door", 0.3f, 1.0f);
            } else {
                stop_loop(i);
            }
        }
    }
}


void unlock_door(int entity) {
    PlayerComponent* player = PlayerComponent_get(entity);
    if (player->target == -1) return;

    DoorComponent* door = DoorComponent_get(player->target);
    if (!door) return;

    if (door->key != -1) {
        for (int i = 0; i < player->keys_size; i++) {
            if (i == NULL_ENTITY) continue;

            ItemComponent* key = ItemComponent_get(player->keys[i]);
            if (key->value == door->key) {
                door->locked = false;
                break;
            }
        }
    }

    if (door->price > 0 && player->money >= door->price) {
        add_money(entity, -door->price);
        door->locked = false;
        door->price = 0;
    }
}
