#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "grid.h"
#include "raycast.h"
#include "util.h"
#include "enemy.h"
#include "physics.h"
#include "sound.h"
#include "collider.h"
#include "particle.h"


int get_akimbo(ComponentData* components, int entity) {
    ItemComponent* item = ItemComponent_get(components, entity);

    if (item->size > 0) {
        if (item->attachments[0] != -1 && components->weapon[item->attachments[0]]) {
            return 1;
        }
    }

    return 0;
}

void reload(ComponentData* component, int i) {
    WeaponComponent* weapon = component->weapon[i];

    if (weapon->max_magazine == -1) {
        return;
    }

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
        if (weapon->cooldown == 0.0f) {
            weapon->cooldown = 1.0f / ((1 + akimbo) * weapon->fire_rate);
            if (weapon->max_magazine != -1) {
                weapon->magazine--;
            }

            sfVector2f pos = get_position(components, parent);
            for (int i = 0; i < weapon->shots; i++) {
                float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
                sfVector2f dir = polar_to_cartesian(1.0, get_angle(components, parent) + angle);
                HitInfo info = raycast(components, grid, pos, dir, weapon->range, parent, BULLETS);

                int dmg = weapon->damage;
                float x = dot(info.normal, dir);
                if (x < -0.99) {
                    dmg *= 4.0;
                }
                damage(components, info.object, info.position, dir, dmg);

                ParticleComponent* particle = ParticleComponent_get(components, entity);
                if (particle) {
                    particle->angle = angle;
                    particle->max_time = dist(pos, info.position) / particle->speed;
                    add_particle(components, entity);
                }
            }

            weapon->recoil = fminf(weapon->max_recoil, weapon->recoil + weapon->recoil_up);

            List* list = get_entities(components, grid, pos, weapon->sound_range);
            for (ListNode* current = list->head; current; current = current->next) {
                int j = current->value;
                if (j == -1) break;

                EnemyComponent* enemy = EnemyComponent_get(components, j);
                if (enemy) {
                    enemy->target = parent;
                    enemy->state = CHASE;
                }
            }
            List_delete(list);

            LightComponent* light = LightComponent_get(components, entity);
            if (light) {
                light->brightness = light->max_brightness;
            }

            if (SoundComponent_get(components, entity)) {
                add_sound(components, entity, "pistol", 1.0, 1.0);
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
    WeaponComponent_add(components, i, 4.0f, 20, 1, 0.0f, 12, 0.25f, 25.0f, 2.0f, false);
    ParticleComponent_add(components, i, 0.0, 0.0, 0.1, 0.1, 100.0, 1, sfWhite, sfWhite)->speed_spread = 0.0;
    ItemComponent_add(components, i, 1);
    SoundComponent_add(components, i, "metal");
    LightComponent_add(components, i, 2.0, 2.0 * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 5.0)->enabled = false;
}


void create_shotgun(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 1.0, 0.5, ITEMS);
    ImageComponent_add(components, i, "pistol", 1.0, 1.0, 3);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    WeaponComponent_add(components, i, 10.0f, 10, 10, 0.1f * M_PI, 2, 0.25f, 25.0f, 1.5f, false);
    ParticleComponent_add(components, i, 0.0, 0.0, 0.05f, 0.05f, 100.0f, 1, sfWhite, sfWhite)->speed_spread = 0.0;
    ItemComponent_add(components, i, 0);
    SoundComponent_add(components, i, "metal");
    LightComponent_add(components, i, 2.0, 2.0 * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 5.0)->enabled = false;
}


void create_axe(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 1.5, 0.5, ITEMS);
    ImageComponent_add(components, i, "axe", 2.0, 1.0, 3);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    WeaponComponent_add(components, i, 2.0f, 100, 10, 0.25f * M_PI, -1, 0.0f, 2.0f, 0.5f, true);
    // ParticleComponent_add(components, i, 0.0, 0.0, 0.1, 0.1, 100.0, 1, sfWhite, sfWhite)->speed_spread = 0.0;
    ItemComponent_add(components, i, 0);
}


void create_lasersight(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);
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
            weapon->recoil -= time_step * weapon->recoil_down;
            weapon->recoil = fmaxf(weapon->recoil, weapon->spread);
            weapon->recoil = fmaxf(0.1f * norm(phys->velocity), weapon->recoil);
        }
    }
}
