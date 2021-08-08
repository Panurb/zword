#define _USE_MATH_DEFINES

#include <math.h>

#include "weapon.h"
#include "component.h"
#include "grid.h"
#include "raycast.h"
#include "util.h"
#include "enemy.h"
#include "physics.h"
#include "sound.h"
#include "collider.h"
#include "particle.h"
#include "health.h"


int get_akimbo(ComponentData* components, int entity) {
    ItemComponent* item = ItemComponent_get(components, entity);

    if (item && item->size > 0) {
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


void create_energy(ComponentData* components, sfVector2f position, sfVector2f velocity) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, position, rand_angle());
    ImageComponent_add(components, i, "energy", 1.0f, 1.0f, LAYER_PARTICLES);
    PhysicsComponent* phys = PhysicsComponent_add(components, i, 0.0f);
    phys->velocity = velocity;
    phys->drag = 0.0f;
    phys->drag_sideways = 0.0f;
    ColliderComponent_add_circle(components, i, 0.25f, GROUP_ENERGY);
    LightComponent_add(components, i, 8.0f, 2.0f * M_PI, get_color(0.5f, 1.0f, 0.0f, 1.0f), 0.5f, 2.5f)->enabled = false;
    ParticleComponent_add_energy(components, i);
    SoundComponent_add(components, i, "");
}


void update_energy(ComponentData* components, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        ColliderComponent* col = ColliderComponent_get(components, i);
        if (!col) continue;
        if (col->group == GROUP_ENERGY) {
            PhysicsComponent* phys = PhysicsComponent_get(components, i);
            LightComponent* light = LightComponent_get(components, i);

            for (ListNode* node = phys->collision.entities->head; node; node = node->next) {
                int j = node->value;
                damage(components, grid, j, get_position(components, i), zeros(), 20);
            }
            
            if (phys->collision.entities->size > 0) {
                light->brightness = 0.5f;
                add_particles(components, i, 10);
                clear_grid(components, grid, i);
                ColliderComponent_remove(components, i);
                ImageComponent_get(components, i)->alpha = 0.0f;
                add_sound(components, i, "energy", 0.5f, randf(1.2f, 1.5f));
                // destroy_entity(components, i);
            }
        }
    }
}


int create_rope(ComponentData* components, sfVector2f start, sfVector2f end) {
    sfVector2f r = diff(end, start);
    float seg_len = 0.5f;
    float len = norm(r);

    int prev = -1;
    int n = len / seg_len;
    for (int i = 0; i < n; i++) {
        int current = create_entity(components);
        sfVector2f pos = sum(start, mult((i * seg_len) / len, r));
        CoordinateComponent_add(components, current, pos, 0.0f);
        PhysicsComponent_add(components, current, 0.2f);
        JointComponent_add(components, current, prev, 0.0f, seg_len, 1.0f);
        ColliderComponent_add_circle(components, current, 0.25f, GROUP_BULLETS);
        ImageComponent_add(components, current, "rope", 0.0f, 0.0f, LAYER_ITEMS);

        prev = current;
    }

    return prev;
}


int rope_root(ComponentData* components, int rope) {
    int j = rope;
    while (true) {
        int p = JointComponent_get(components, j)->parent;
        if (p == -1) break;
        j = p;
    }

    return j;
}


void shoot(ComponentData* components, ColliderGrid* grid, int entity) {
    WeaponComponent* weapon = WeaponComponent_get(components, entity);
    int parent = CoordinateComponent_get(components, entity)->parent;

    int akimbo = get_akimbo(components, entity);

    if (weapon->magazine > 0) {
        if (weapon->cooldown == 0.0f) {
            weapon->cooldown = 1.0f / ((1 + akimbo) * weapon->fire_rate);
            if (weapon->max_magazine != -1) {
                weapon->magazine--;
            }

            sfVector2f pos = get_position(components, parent);

            switch (weapon->ammo_type) {
                case AMMO_MELEE: {
                    HitInfo min_info;
                    float min_dist = INFINITY;
                    for (int i = 0; i < weapon->shots; i++) {
                        float angle = i * weapon->spread / (weapon->shots - 1) - 0.5f * weapon->spread;
                        sfVector2f dir = polar_to_cartesian(1.0, get_angle(components, parent) + angle);
                        HitInfo info = raycast(components, grid, pos, dir, weapon->range, GROUP_BULLETS);

                        float d = dist(info.position, pos);
                        if (d < min_dist) {
                            min_info = info;
                            min_dist = d;
                        }
                    }

                    int dmg = weapon->damage;
                    EnemyComponent* enemy = EnemyComponent_get(components, min_info.entity);
                    if (enemy && enemy->state == ENEMY_IDLE) {
                        dmg = 100;
                    }
                    damage(components, grid, min_info.entity, min_info.position, normalized(diff(min_info.position, pos)), dmg);
                    if (min_info.entity != -1) {
                        shake_camera(components, 0.0125f * weapon->damage);
                    }
                    break;
                } case AMMO_ENERGY: {
                    for (int i = 0; i < weapon->shots; i++) {
                        float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
                        if (weapon->spread > 0.0f) {
                            angle = i * weapon->spread / (weapon->shots - 1) - 0.5f * weapon->spread + randf(-0.1f, 0.1f);
                        }
                        sfVector2f vel = polar_to_cartesian(7.0f, get_angle(components, parent) + angle);
                        create_energy(components, sum(get_position(components, entity), mult(1.0f / 14.0f, vel)), vel);
                    }
                    break;
                } case AMMO_ROPE: {
                    float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
                    sfVector2f dir = polar_to_cartesian(1.0f, get_angle(components, parent) + angle);
                    HitInfo info = raycast(components, grid, pos, dir, weapon->range, GROUP_BULLETS);

                    int i = create_rope(components, info.position, get_position(components, parent));

                    int j = rope_root(components, i);
                    JointComponent_get(components, j)->parent = info.entity;
                    // CoordinateComponent* coord = CoordinateComponent_get(components, j);
                    // coord->parent = j;
                    // coord->
                    // PhysicsComponent_remove(components, j);

                    damage(components, grid, info.entity, info.position, dir, weapon->damage);

                    break;
                } default: {
                    ParticleComponent* particle = ParticleComponent_get(components, entity);
                    for (int i = 0; i < weapon->shots; i++) {
                        float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
                        if (weapon->spread > 0.0f) {
                            angle = i * weapon->spread / (weapon->shots - 1) - 0.5f * weapon->spread + randf(-0.1f, 0.1f);
                        }
                        sfVector2f dir = polar_to_cartesian(1.0, get_angle(components, parent) + angle);
                        HitInfo info = raycast(components, grid, pos, dir, weapon->range, GROUP_BULLETS);

                        int dmg = weapon->damage;
                        if (dot(info.normal, dir) < -0.99f) {
                            dmg *= 2;
                        }
                        damage(components, grid, info.entity, info.position, dir, dmg);

                        particle->angle = angle;
                        if (info.entity) {
                            particle->max_time = 0.9f * dist(pos, info.position) / particle->speed;
                        } else {
                            particle->max_time = weapon->range / particle->speed;
                        }
                        add_particles(components, entity, 1);
                    }

                    weapon->recoil = fminf(weapon->max_recoil, weapon->recoil + weapon->recoil_up);
                    if (PlayerComponent_get(components, parent)) {
                        alert_enemies(components, grid, parent, weapon->sound_range);
                    }

                    LightComponent* light = LightComponent_get(components, entity);
                    if (light) {
                        light->brightness = light->max_brightness;
                    }

                    shake_camera(components, 0.025f * weapon->shots * weapon->damage);
                }
            }

            SoundComponent* sound = SoundComponent_get(components, entity);
            if (sound) {
                add_sound(components, entity, weapon->sound, 1.0f, 1.0f);
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


int create_pistol(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(components, i, "pistol", 1.0, 1.0, LAYER_ITEMS);
    PhysicsComponent_add(components, i, 0.5f);
    WeaponComponent_add(components, i, 10.0f, 20, 1, 0.0f, 12, 0.1f, 25.0f, 2.0f, AMMO_PISTOL, "pistol");
    ParticleComponent_add_bullet(components, i, 0.15f);
    ItemComponent_add(components, i, 1);
    SoundComponent_add(components, i, "metal");
    LightComponent_add(components, i, 2.0, 2.0 * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 10.0f)->enabled = false;

    return i;
}


int create_shotgun(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(components, i, "shotgun", 2.0, 1.0, 3);
    PhysicsComponent_add(components, i, 0.5f);
    WeaponComponent_add(components, i, 10.0f, 10, 10, 0.1f * M_PI, 2, 0.25f, 20.0f, 1.5f, AMMO_SHOTGUN, "shotgun");
    ParticleComponent_add_bullet(components, i, 0.1f);
    ItemComponent_add(components, i, 0);
    SoundComponent_add(components, i, "metal");
    LightComponent_add(components, i, 3.0f, 2.0 * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 5.0f)->enabled = false;

    return i;
}


int create_rifle(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(components, i, "assault_rifle", 3.0f, 1.0f, LAYER_ITEMS);
    PhysicsComponent_add(components, i, 0.5f);
    WeaponComponent_add(components, i, 0.5f, 10, 1, 0.0f, -1, 0.05f, 30.0f, 3.0f, AMMO_RIFLE, "assault_rifle")->automatic = true;
    ParticleComponent_add_bullet(components, i, 0.15f);
    ItemComponent_add(components, i, 1);
    SoundComponent_add(components, i, "metal");
    LightComponent_add(components, i, 4.0f, 2.0f * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 10.0f)->enabled = false;

    return i;
}


int create_assault_rifle(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(components, i, "assault_rifle", 3.0f, 1.0f, LAYER_ITEMS);
    PhysicsComponent_add(components, i, 0.5f);
    WeaponComponent_add(components, i, 10.0f, 40, 1, 0.0f, 30, 0.05f, 30.0f, 3.0f, AMMO_RIFLE, "assault_rifle")->automatic = true;
    ParticleComponent_add_bullet(components, i, 0.15f);
    ItemComponent_add(components, i, 1);
    SoundComponent_add(components, i, "metal");
    LightComponent_add(components, i, 4.0f, 2.0f * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 10.0f)->enabled = false;

    return i;
}


int create_axe(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(components, i, "axe", 2.0, 1.0, LAYER_ITEMS);
    PhysicsComponent_add(components, i, 1.0f);
    WeaponComponent_add(components, i, 2.0f, 50, 7, 0.35f * M_PI, -1, 0.0f, 2.0f, 0.0f, AMMO_MELEE, "axe");
    ItemComponent_add(components, i, 0);
    SoundComponent_add(components, i, "metal");

    return i;
}


int create_rope_gun(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_circle(components, i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(components, i, "pistol", 1.0, 1.0, LAYER_ITEMS);
    PhysicsComponent_add(components, i, 0.5f);
    WeaponComponent_add(components, i, 0.5f, 0, 1, 0.0f, -1, 0.0f, 25.0f, 2.0f, AMMO_ROPE, "pistol");
    ItemComponent_add(components, i, 0);
    SoundComponent_add(components, i, "metal");

    return i;
}


int create_lasersight(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, rand_angle());
    PhysicsComponent_add(components, i, 0.5f);
    ItemComponent_add(components, i, 0);
    LightComponent_add(components, i, 20.0, 0.01, sfRed, 1.0, 10.0)->enabled = false;
    ImageComponent_add(components, i, "flashlight", 1.0, 1.0, LAYER_ITEMS);

    return i;
}


int create_ammo(ComponentData* components, sfVector2f position, AmmoType type) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_circle(components, i, 0.25f, GROUP_ITEMS);
    switch (type) {
        case AMMO_PISTOL:
            ImageComponent_add(components, i, "ammo_pistol", 1.0, 1.0, LAYER_ITEMS);
            break;
        case AMMO_RIFLE:
            ImageComponent_add(components, i, "ammo_rifle", 1.0, 1.0, LAYER_ITEMS);
            break;
        case AMMO_SHOTGUN:
            ImageComponent_add(components, i, "ammo_shotgun", 1.0, 1.0, LAYER_ITEMS);
            break;
        default:
            break;
    }
    AmmoComponent_add(components, i, type);
    PhysicsComponent_add(components, i, 0.5f);
    ItemComponent_add(components, i, 0);
    SoundComponent_add(components, i, "metal");
    return i;
}
