#include "component.h"
#include "light.h"
#include "grid.h"
#include "util.h"


void update_enemy(Component* component, ColliderGrid* grid) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->enemy[i]) continue;

        CoordinateComponent* coord = component->coordinate[i];
        EnemyComponent* enemy = component->enemy[i];
        PhysicsComponent* phys = component->physics[i];

        if (enemy->health <= 0) {
            destroy_entity(component, i);
            break;
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
