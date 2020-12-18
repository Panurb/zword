#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "grid.h"
#include "raycast.h"
#include "util.h"
#include "enemy.h"
#include "physics.h"


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


void shoot(ComponentData* components, ColliderGrid* grid, int entity) {
    WeaponComponent* weapon = components->weapon[entity];
    int parent = components->coordinate[entity]->parent;

    int akimbo = get_akimbo(components, entity);

    if (weapon->magazine > 0) {
        if (weapon->cooldown == 0.0) {
            weapon->cooldown = 1.0 / ((1 + akimbo) * weapon->fire_rate);
            weapon->magazine--;

            float angle = randf(-0.5 * weapon->recoil, 0.5 * weapon->recoil);
            sfVector2f r = polar_to_cartesian(1.0, get_angle(components, parent) +  angle);

            sfVector2f pos = get_position(components, parent);

            HitInfo info = raycast(components, grid, pos, r, 20.0, parent);

            if (components->health[info.object]) {
                float x = dot(info.normal, normalized(r));
                float dmg = weapon->damage;
                if (x < -0.99) {
                    dmg *= 4.0;
                }

                damage(components, info.object, dmg, parent);
            }
            
            PhysicsComponent* physics = PhysicsComponent_get(components, info.object);
            if (physics) {
                apply_force(components, info.object, mult(250.0, r));
            }

            ParticleComponent* particle = ParticleComponent_get(components, info.object);
            if (particle) {
                particle->origin = diff(info.position, get_position(components, info.object));
                particle->enabled = true;
            }

            weapon->recoil = fmin(weapon->max_recoil, weapon->recoil + weapon->recoil_up);

            particle = ParticleComponent_get(components, entity);
            particle->angle = angle;
            particle->max_time = dist(pos, info.position) / particle->speed;
            particle->enabled = true;

            int entities[100];
            get_entities(components, grid, pos, 20.0, entities);

            for (int i = 0; i < 100; i++) {
                int j = entities[i];
                if (j == -1) break;

                EnemyComponent* enemy = EnemyComponent_get(components, j);
                if (enemy) {
                    enemy->target = parent;
                    enemy->state = CHASE;
                }
            }
        }
    } else {
        reload(components, entity);
    }
}


void create_pistol(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 1.0, 0.5, ITEMS);
    ImageComponent_add(components, i, "pistol", 1.0, 1.0, 3);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    WeaponComponent_add(components, i, 4.0, 20, 12, 0.25, 0.75, 0.25 * M_PI);
    ParticleComponent_add(components, i, 0.0, 0.0, 0.1, 0.1, 100.0, 1, sfWhite, sfWhite)->speed_spread = 0.0;
    ItemComponent_add(components, i, 1);
}


void create_lasersight(ComponentData* components, float x, float y) {
    int i = create_entity(components);

    sfVector2f pos = { x, y };

    CoordinateComponent_add(components, i, pos, rand_angle());
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    ItemComponent_add(components, i, 0);
    LightComponent_add(components, i, 20.0, 0.01, sfRed, 1.0, 10.0)->enabled = false;
    ImageComponent_add(components, i, "zombie", 1.0, 1.0, 3);
}


void update_weapons(ComponentData* components, float time_step) {
    for (int i = 0; i < components->entities; i++) {
        WeaponComponent* weapon = components->weapon[i];
        if (!weapon) continue;

        weapon->cooldown = fmax(0.0, weapon->cooldown - time_step);

        int parent = CoordinateComponent_get(components, i)->parent;
        if (parent != -1) {
            PhysicsComponent* phys = PhysicsComponent_get(components, parent);
            weapon->recoil = fmax(0.1 * norm(phys->velocity), weapon->recoil - time_step * weapon->recoil_down);
        }
    }
}
