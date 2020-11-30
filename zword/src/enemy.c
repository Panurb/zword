#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "raycast.h"
#include "grid.h"
#include "util.h"


void update_enemy(Component* component, ColliderGrid* grid) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->enemy[i]) continue;

        CoordinateComponent* coord = component->coordinate[i];
        EnemyComponent* enemy = component->enemy[i];
        PhysicsComponent* phys = component->physics[i];

        if (enemy->health <= 0) {
            component->circle_collider[i]->enabled = false;
            continue;
        }

        if (enemy->target == -1) {
            for (int j = 0; j < component->entities; j++) {
                if (!component->player[j]) continue;

                sfVector2f velocity = diff(component->coordinate[j]->position, coord->position);
                if (raycast(component, grid, coord->position, velocity, 20.0).object == j) {
                    enemy->target = j;
                    break;
                }
            }
        } else {
            sfVector2f r = diff(component->coordinate[enemy->target]->position, coord->position);
            phys->acceleration = sum(phys->acceleration, mult(enemy->acceleration, normalized(r)));
        }
    }
}


void create_enemy(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->image[i] = ImageComponent_create("player", 1.0);
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
    component->physics[i] = PhysicsComponent_create(1.0, 0.0, 0.25, 5.0);
    component->physics[i]->max_speed = 2.0;
    component->enemy[i] = EnemyComponent_create();
    component->particle[i] = ParticleComponent_create(0.0, 2 * M_PI, 0.5, 0.0, 5.0, 10.0, sfRed, sfColor_fromRGB(255, 150, 150));
}
