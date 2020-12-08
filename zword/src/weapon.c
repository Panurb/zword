#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "grid.h"
#include "raycast.h"
#include "util.h"


void reload(Component* component, int i) {
    WeaponComponent* weapon = component->weapon[i];
    if (!weapon->reloading) {
        weapon->cooldown = weapon->reload_time;
        weapon->reloading = true;
    }
}


void shoot(Component* component, ColliderGrid* grid, int i) {
    WeaponComponent* weapon = component->weapon[i];
    int parent = component->coordinate[i]->parent;

    if (weapon->magazine > 0) {
        if (weapon->cooldown == 0.0) {
            weapon->cooldown = 1.0 / weapon->fire_rate;
            weapon->magazine--;

            float angle = float_rand(-0.5 * weapon->recoil, 0.5 * weapon->recoil);
            sfVector2f r = polar_to_cartesian(0.6, get_angle(component, parent) +  angle);

            sfVector2f pos = sum(get_position(component, parent), r);

            HitInfo info = raycast(component, grid, pos, r, 20.0, parent);

            if (component->enemy[info.object]) {
                component->enemy[info.object]->health -= weapon->damage;
                component->physics[info.object]->velocity = polar_to_cartesian(2.0, get_angle(component, parent) +  angle);

                component->particle[info.object]->enabled = true;
            }

            weapon->recoil = fmin(weapon->max_recoil, weapon->recoil + weapon->recoil_up);
            component->particle[i]->angle = angle;
            component->particle[i]->max_time = dist(pos, info.position) / component->particle[i]->speed;
            component->particle[i]->enabled = true;
        }
    } else {
        reload(component, i);
    }
}


void create_weapon(Component* component, float x, float y) {
    int i = get_index(component);

    sfVector2f pos = { x, y };
    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->collider[i] = ColliderComponent_create_rectangle(1.5, 0.25);
    component->physics[i] = PhysicsComponent_create(0.5, 0.0, 0.5, 10.0, 2.5);
    component->weapon[i] = WeaponComponent_create(5.0, 20, 30, 0.25, 0.75, 0.5 * M_PI);
    component->particle[i] = ParticleComponent_create(0.0, 0.0, 0.1, 0.1, 100.0, 1, sfWhite, sfWhite);
    component->particle[i]->speed_spread = 0.0;
    component->item[i] = ItemComponent_create(1);
}


void create_lasersight(Component* component, float x, float y) {
    int i = get_index(component);

    sfVector2f pos = { x, y };
    component->coordinate[i] = CoordinateComponent_create(pos, float_rand(0.0, 2 * M_PI));
    component->collider[i] = ColliderComponent_create_rectangle(1.0, 0.25);
    component->physics[i] = PhysicsComponent_create(0.5, 0.0, 0.5, 10.0, 2.5);
    component->item[i] = ItemComponent_create(0);
    component->light[i] = LightComponent_create(20.0, 0.01, 1, sfRed, 1.0, 10.0);
    component->light[i]->enabled = false;
}
