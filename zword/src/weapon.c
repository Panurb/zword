#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "grid.h"
#include "raycast.h"
#include "util.h"
#include "enemy.h"


int get_akimbo(ComponentData* components, int entity) {
    ItemComponent* item = components->item[entity];
    if (item->attachments[0] != -1 && components->weapon[item->attachments[0]]) {
        return 1;
    }
    return 0;
}

void reload(ComponentData* component, int i) {
    WeaponComponent* weapon = component->weapon[i];

    int akimbo = get_akimbo(component, i);

    if (weapon->reloading) {
        if (weapon->cooldown == 0.0) {
            weapon->magazine = (1 + akimbo) * weapon->max_magazine;
            weapon->reloading = false;
        }
    } else {
        if (weapon->magazine != weapon->max_magazine) {
            weapon->cooldown = (1 + akimbo) * weapon->reload_time;
            weapon->reloading = true;
        }
    }
}


void shoot(ComponentData* component, ColliderGrid* grid, int i) {
    WeaponComponent* weapon = component->weapon[i];
    int parent = component->coordinate[i]->parent;

    int akimbo = get_akimbo(component, i);

    if (weapon->magazine > 0) {
        if (weapon->cooldown == 0.0) {
            weapon->cooldown = 1.0 / ((1 + akimbo) * weapon->fire_rate);
            weapon->magazine--;

            float angle = float_rand(-0.5 * weapon->recoil, 0.5 * weapon->recoil);
            sfVector2f r = polar_to_cartesian(1.0, get_angle(component, parent) +  angle);

            sfVector2f pos = get_position(component, parent);

            HitInfo info = raycast(component, grid, pos, r, 20.0, parent);

            if (component->health[info.object]) {
                float x = dot(info.normal, normalized(r));
                float dmg = weapon->damage;
                if (x < -0.99) {
                    dmg *= 2.0;
                }

                damage(component, info.object, dmg, parent);
            }
            
            if (component->physics[info.object]) {
                component->physics[info.object]->velocity = polar_to_cartesian(2.0, get_angle(component, parent) +  angle);
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


void create_weapon(ComponentData* components, float x, float y) {
    int i = get_index(components);

    sfVector2f pos = { x, y };

    CoordinateComponent_add(components, i, pos, float_rand(0.0, 2 * M_PI));
    ColliderComponent_add_rectangle(components, i, 0.5, 0.25, ITEMS);
    ImageComponent_add(components, i, "pistol", 1.0, 1.0, 3);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    components->weapon[i] = WeaponComponent_create(4.0, 20, 12, 0.25, 0.75, 0.25 * M_PI);
    components->particle[i] = ParticleComponent_create(0.0, 0.0, 0.1, 0.1, 100.0, 1, sfWhite, sfWhite);
    components->particle[i]->speed_spread = 0.0;
    ItemComponent_add(components, i, 1);
}


void create_lasersight(ComponentData* components, float x, float y) {
    int i = get_index(components);

    sfVector2f pos = { x, y };

    CoordinateComponent_add(components, i, pos, float_rand(0.0, 2 * M_PI));
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    ItemComponent_add(components, i, 0);
    components->light[i] = LightComponent_create(20.0, 0.01, 1, sfRed, 1.0, 10.0);
    components->light[i]->enabled = false;
    ImageComponent_add(components, i, "zombie", 1.0, 1.0, 3);
}


void update_weapons(ComponentData* components, float time_step) {
    for (int i = 0; i < components->entities; i++) {
        WeaponComponent* weapon = components->weapon[i];
        if (!weapon) continue;

        weapon->cooldown = fmax(0.0, weapon->cooldown - time_step);

        int parent = components->coordinate[i]->parent;
        if (parent != -1) {
            PhysicsComponent* phys = components->physics[parent];
            weapon->recoil = fmax(0.1 * norm(phys->velocity), weapon->recoil - time_step * weapon->recoil_down);
        }
    }
}
