#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "grid.h"
#include "raycast.h"
#include "util.h"


void shoot(Component* component, ColliderGrid* grid, int i, float delta_time) {
    PlayerComponent* player = component->player[i];

    int item = player->inventory[player->item];
    WeaponComponent* weapon = component->weapon[item];

    if (weapon->cooldown <= 0.0) {
        weapon->cooldown = 1.0 / weapon->fire_rate;

        float angle = float_rand(-0.5 * weapon->recoil, 0.5 * weapon->recoil);
        sfVector2f r = polar_to_cartesian(0.6, component->coordinate[i]->angle + angle);

        sfVector2f pos = sum(component->coordinate[i]->position, r);

        HitInfo info = raycast(component, grid, pos, r, 20.0);

        if (component->enemy[info.object]) {
            component->enemy[info.object]->health -= 10;
            component->physics[info.object]->velocity = polar_to_cartesian(2.0, component->coordinate[i]->angle + angle);

            component->particle[info.object]->enabled = true;
        }

        weapon->recoil = fmin(0.5 * M_PI, weapon->recoil + delta_time * weapon->recoil_up);
        component->particle[item]->angle = angle;
        component->particle[item]->max_time = dist(pos, info.position) / component->particle[item]->speed;
        component->particle[item]->enabled = true;
    }
}


void create_weapon(Component* component, float x, float y) {
    int i = component->entities;
    component->entities++;

    sfVector2f pos = { x, y };
    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->rectangle_collider[i] = RectangleColliderComponent_create(1.5, 0.25);
    component->physics[i] = PhysicsComponent_create(0.5, 0.0, 0.5, 10.0, 2.5);
    component->weapon[i] = WeaponComponent_create(5.0, 15.0, 0.75);
    component->particle[i] = ParticleComponent_create(0.0, 0.0, 0.1, 0.1, 100.0, 1, sfWhite, sfWhite);
    component->particle[i]->speed_spread = 0.0;
    component->item[i] = ItemComponent_create(1);
    component->light[i] = LightComponent_create(20.0, 0.01, 1, sfRed, 1.0, 10.0);
    component->light[i]->enabled = false;
}
