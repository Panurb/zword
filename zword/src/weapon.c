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

void reload(ComponentData* components, int i) {
    WeaponComponent* weapon = components->weapon[i];
    if (weapon->max_magazine == -1) {
        return;
    }

    PlayerComponent* player = PlayerComponent_get(components, CoordinateComponent_get(components, i)->parent);
    AmmoComponent* ammo = AmmoComponent_get(components, player->ammo[weapon->ammo_type]);
    if (ammo->size == 0) {
        return;
    }

    int akimbo = get_akimbo(components, i);

    if (weapon->reloading) {
        if (weapon->cooldown == 0.0f) {
            int a = fminf(ammo->size, (1 + akimbo) * (weapon->max_magazine - weapon->magazine));
            weapon->magazine += a;
            ammo->size -= a;
            weapon->reloading = false;
        }
    } else {
        if (weapon->magazine < weapon->max_magazine) {
            weapon->cooldown = (1 + akimbo) * weapon->reload_time;
            weapon->reloading = true;
        }
    }
}


void shoot(ComponentData* components, ColliderGrid* grid, int entity) {
    WeaponComponent* weapon = components->weapon[entity];
    int parent = CoordinateComponent_get(components, entity)->parent;

    int akimbo = get_akimbo(components, entity);

    if (weapon->magazine > 0) {
        if (weapon->cooldown == 0.0f) {
            weapon->cooldown = 1.0f / ((1 + akimbo) * weapon->fire_rate);
            if (weapon->max_magazine != -1) {
                weapon->magazine--;
            }

            sfVector2f pos = get_position(components, parent);

            if (weapon->ammo_type == AMMO_MELEE) {
                HitInfo min_info;
                float min_dist = INFINITY;
                for (int i = 0; i < weapon->shots; i++) {
                    float angle = i * weapon->spread / (weapon->shots - 1) - 0.5f * weapon->spread;
                    sfVector2f dir = polar_to_cartesian(1.0, get_angle(components, parent) + angle);
                    HitInfo info = raycast(components, grid, pos, dir, weapon->range, parent, BULLETS);

                    float d = dist(info.position, pos);
                    if (d < min_dist) {
                        min_info = info;
                        min_dist = d;
                    }
                }

                int dmg = weapon->damage;
                EnemyComponent* enemy = EnemyComponent_get(components, min_info.object);
                if (enemy && enemy->state == IDLE) {
                    dmg *= 2;
                }
                damage(components, min_info.object, min_info.position, normalized(diff(min_info.position, pos)), dmg);
            } else {
                ParticleComponent* particle = ParticleComponent_get(components, entity);
                for (int i = 0; i < weapon->shots; i++) {
                    float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
                    if (weapon->spread > 0.0f) {
                        angle = i * weapon->spread / (weapon->shots - 1) - 0.5f * weapon->spread + randf(-0.1f, 0.1f);
                    }
                    sfVector2f dir = polar_to_cartesian(1.0, get_angle(components, parent) + angle);
                    HitInfo info = raycast(components, grid, pos, dir, weapon->range, parent, BULLETS);

                    int dmg = weapon->damage;
                    if (dot(info.normal, dir) < -0.99f) {
                        dmg *= 4;
                    }
                    damage(components, info.object, info.position, dir, dmg);

                    particle->angle = angle;
                    if (info.object) {
                        particle->max_time = 0.9f * dist(pos, info.position) / particle->speed;
                    } else {
                        particle->max_time = weapon->range / particle->speed;
                    }
                    add_particles(components, entity, 1);
                }

                weapon->recoil = fminf(weapon->max_recoil, weapon->recoil + weapon->recoil_up);
                alert_enemies(components, grid, parent, 30.0f);

                LightComponent* light = LightComponent_get(components, entity);
                if (light) {
                    light->brightness = light->max_brightness;
                }
            }

            SoundComponent* sound = SoundComponent_get(components, entity);
            if (sound) {
                add_sound(components, entity, weapon->sound, 1.0, 1.0);
            }
        }
    }
    
    if (weapon->magazine == 0) {
        reload(components, entity);
    }
}


void update_weapons(ComponentData* components, float time_step) {
    for (int i = 0; i < components->entities; i++) {
        WeaponComponent* weapon = components->weapon[i];
        if (!weapon) continue;

        weapon->cooldown = fmax(0.0f, weapon->cooldown - time_step);

        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        int parent = coord->parent;
        if (parent != -1) {
            PhysicsComponent* phys = PhysicsComponent_get(components, parent);
            weapon->recoil -= time_step * weapon->recoil_down;
            weapon->recoil = fmaxf(weapon->recoil_up * phys->speed / phys->max_speed, weapon->recoil);

            if (weapon->ammo_type == AMMO_MELEE) {
                float x_max = 1.0f / weapon->fire_rate;
                float x = 2.0f * (x_max - weapon->cooldown) / x_max;
                coord->angle = M_PI * powf(x - 1.0f, 2.0f) - 0.45f * M_PI;
            }
        }
    }
}


void create_pistol(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 1.0, 0.5, ITEMS);
    ImageComponent_add(components, i, "pistol", 1.0, 1.0, 3);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    WeaponComponent_add(components, i, 10.0f, 20, 1, 0.0f, 12, 0.1f, 25.0f, 2.0f, AMMO_PISTOL, "pistol");
    ParticleComponent_add(components, i, 0.0, 0.0, 0.1, 0.1, 100.0, 1, sfWhite, sfWhite)->speed_spread = 0.0;
    ItemComponent_add(components, i, 1);
    SoundComponent_add(components, i, "metal");
    LightComponent_add(components, i, 2.0, 2.0 * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 10.0f)->enabled = false;
}


void create_shotgun(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 1.0, 0.5, ITEMS);
    ImageComponent_add(components, i, "shotgun", 2.0, 1.0, 3);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    WeaponComponent_add(components, i, 10.0f, 10, 10, 0.1f * M_PI, 2, 0.25f, 15.0f, 1.5f, AMMO_SHOTGUN, "shotgun");
    ParticleComponent_add(components, i, 0.0, 0.0, 0.05f, 0.05f, 100.0f, 1, sfWhite, sfWhite)->speed_spread = 0.0;
    ItemComponent_add(components, i, 0);
    SoundComponent_add(components, i, "metal");
    LightComponent_add(components, i, 3.0f, 2.0 * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 5.0f)->enabled = false;
}


void create_assault_rifle(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 1.0f, 0.5f, ITEMS);
    ImageComponent_add(components, i, "assault_rifle", 3.0f, 1.0f, 3);
    PhysicsComponent_add(components, i, 0.5f, 0.0f, 0.5f, 10.0f, 2.5f);
    WeaponComponent_add(components, i, 10.0f, 40, 1, 0.0f, 30, 0.05f, 30.0f, 3.0f, AMMO_RIFLE, "assault_rifle")->automatic = true;
    ParticleComponent_add(components, i, 0.0f, 0.0f, 0.125f, 0.125f, 100.0f, 1, sfWhite, sfWhite)->speed_spread = 0.0f;
    ItemComponent_add(components, i, 1);
    SoundComponent_add(components, i, "metal");
    LightComponent_add(components, i, 4.0f, 2.0f * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 10.0f)->enabled = false;
}


void create_axe(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 1.5, 0.5, ITEMS);
    ImageComponent_add(components, i, "axe", 2.0, 1.0, 3);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    WeaponComponent_add(components, i, 2.0f, 50, 7, 0.35f * M_PI, -1, 0.0f, 2.0f, 0.5f, AMMO_MELEE, "axe");
    ItemComponent_add(components, i, 0);
    SoundComponent_add(components, i, "metal");
}


void create_lasersight(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, rand_angle());
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    ItemComponent_add(components, i, 0);
    LightComponent_add(components, i, 20.0, 0.01, sfRed, 1.0, 10.0)->enabled = false;
    ImageComponent_add(components, i, "flashlight", 1.0, 1.0, 3);
}


int create_ammo(ComponentData* components, sfVector2f position, AmmoType type) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 0.5f, 0.5f, ITEMS);
    switch (type) {
        case AMMO_PISTOL:
            ImageComponent_add(components, i, "ammo_pistol", 1.0, 1.0, 3);
            break;
        case AMMO_RIFLE:
            ImageComponent_add(components, i, "ammo_rifle", 1.0, 1.0, 3);
            break;
        case AMMO_SHOTGUN:
            ImageComponent_add(components, i, "ammo_shotgun", 1.0, 1.0, 3);
            break;
        default:
            break;
    }
    AmmoComponent_add(components, i, type);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    ItemComponent_add(components, i, 0);
    SoundComponent_add(components, i, "metal");
    return i;
}
