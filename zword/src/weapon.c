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
#include "image.h"


int get_akimbo(ComponentData* components, int entity) {
    ItemComponent* item = ItemComponent_get(entity);

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

    PlayerComponent* player = PlayerComponent_get(CoordinateComponent_get(i)->parent);
    AmmoComponent* ammo = AmmoComponent_get(player->ammo[weapon->ammo_type]);
    if (!ammo || ammo->size == 0) {
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
    int i = create_entity();
    CoordinateComponent_add(i, position, rand_angle());
    ImageComponent_add(i, "energy", 1.0f, 1.0f, LAYER_PARTICLES);
    PhysicsComponent* phys = PhysicsComponent_add(i, 0.0f);
    phys->velocity = velocity;
    phys->drag = 0.0f;
    phys->drag_sideways = 0.0f;
    phys->lifetime = 5.0f;
    ColliderComponent_add_circle(i, 0.25f, GROUP_ENERGY);
    LightComponent_add(i, 8.0f, 2.0f * M_PI, COLOR_ENERGY, 0.5f, 2.5f)->enabled = false;
    ParticleComponent_add_type(i, PARTICLE_ENERGY, 0.0f);
    SoundComponent_add(i, "");
}


void update_energy(ComponentData* components, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        ColliderComponent* col = ColliderComponent_get(i);
        if (!col) continue;
        if (col->group == GROUP_ENERGY) {
            PhysicsComponent* phys = PhysicsComponent_get(i);
            LightComponent* light = LightComponent_get(i);

            for (ListNode* node = phys->collision.entities->head; node; node = node->next) {
                int j = node->value;
                damage(components, grid, j, get_position(i), zeros(), 20, -1);
            }
            
            if (phys->collision.entities->size > 0) {
                light->brightness = 0.5f;
                add_particles(i, 10);
                clear_grid(i);
                ColliderComponent_get(i)->enabled = false;
                ImageComponent_remove(i);
                add_sound(components, i, "energy", 0.5f, randf(1.2f, 1.5f));
                phys->lifetime = 1.0f;
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
        int current = create_entity();
        sfVector2f pos = sum(start, mult((i * seg_len) / len, r));
        CoordinateComponent_add(current, pos, 0.0f);
        PhysicsComponent_add(current, 0.2f);
        JointComponent_add(current, prev, 0.0f, seg_len, 1.0f);
        ColliderComponent_add_circle(current, 0.25f, GROUP_BULLETS);
        ImageComponent_add(current, "rope", 0.0f, 0.0f, LAYER_ITEMS);

        prev = current;
    }

    return prev;
}


int rope_root(ComponentData* components, int rope) {
    int j = rope;
    while (true) {
        int p = JointComponent_get(j)->parent;
        if (p == -1) break;
        j = p;
    }

    return j;
}


void attack(ComponentData* components, ColliderGrid* grid, int entity) {
    WeaponComponent* weapon = WeaponComponent_get(entity);
    int parent = CoordinateComponent_get(entity)->parent;

    if (weapon->magazine == 0) {
        reload(components, entity);
        return;
    }

    if (weapon->cooldown != 0.0f) {
        return;
    }

    int akimbo = get_akimbo(components, entity);
    weapon->cooldown = 1.0f / ((1 + akimbo) * weapon->fire_rate);
    if (weapon->max_magazine != -1) {
        weapon->magazine--;
    }

    PlayerComponent* player = PlayerComponent_get(parent);
    if (player) {
        AnimationComponent_get(player->arms)->framerate = 20.0f;
    }

    sfVector2f pos = get_position(parent);

    switch (weapon->ammo_type) {
        case AMMO_MELEE: {
            HitInfo min_info[weapon->shots];
            float min_dist[weapon->shots];
            for (int i = 0; i < weapon->shots; i++) {
                min_info[i].entity = -1;
                min_dist[i] = INFINITY;
            }

            int rays = 7;
            for (int i = 0; i < rays; i++) {
                float angle = i * weapon->spread / (rays - 1) - 0.5f * weapon->spread;
                sfVector2f dir = polar_to_cartesian(1.0, get_angle(parent) + angle);
                HitInfo info = raycast(pos, dir, weapon->range, GROUP_BULLETS);

                float d = dist(info.position, pos);

                int max_index = -1;
                for (int j = 0; j < weapon->shots; j++) {
                    if (min_info[j].entity == info.entity) {
                        max_index = j;
                        break;
                    }
                }

                if (max_index == -1) {
                    max_index = argmax(min_dist, weapon->shots);
                }
                
                if (d < min_dist[max_index]) {
                    min_info[max_index] = info;
                    min_dist[max_index] = d;
                }
            }

            int dmg = weapon->damage;
            for (int i = 0; i < weapon->shots; i++) {
                if (min_info[i].entity == -1) continue;

                EnemyComponent* enemy = EnemyComponent_get(min_info[i].entity);
                if (enemy && enemy->state == ENEMY_IDLE) {
                    dmg *= 2;
                }
                sfVector2f dir = normalized(diff(min_info[i].position, pos));
                damage(components, grid, min_info[i].entity, min_info[i].position, dir, dmg, parent);
                shake_camera(0.0125f * weapon->damage);
            }
            break;
        } case AMMO_ENERGY: {
            for (int i = 0; i < weapon->shots; i++) {
                float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
                if (weapon->spread > 0.0f) {
                    angle = i * weapon->spread / (weapon->shots - 1) - 0.5f * weapon->spread + randf(-0.1f, 0.1f);
                }
                sfVector2f vel = polar_to_cartesian(7.0f, get_angle(parent) + angle);
                create_energy(components, sum(get_position(entity), mult(1.0f / 14.0f, vel)), vel);
            }
            break;
        } case AMMO_ROPE: {
            float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
            sfVector2f dir = polar_to_cartesian(1.0f, get_angle(parent) + angle);
            HitInfo info = raycast(pos, dir, weapon->range, GROUP_BULLETS);

            int i = create_rope(components, info.position, get_position(parent));

            int j = rope_root(components, i);
            JointComponent_get(j)->parent = info.entity;
            // CoordinateComponent* coord = CoordinateComponent_get(j);
            // coord->parent = j;
            // coord->
            // PhysicsComponent_remove(j);

            damage(components, grid, info.entity, info.position, dir, weapon->damage, parent);

            break;
        } default: {
            ParticleComponent* particle = ParticleComponent_get(entity);
            for (int i = 0; i < weapon->shots; i++) {
                float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
                if (weapon->spread > 0.0f) {
                    angle = i * weapon->spread / (weapon->shots - 1) - 0.5f * weapon->spread + randf(-0.1f, 0.1f);
                }
                sfVector2f dir = polar_to_cartesian(1.0, get_angle(parent) + angle);
                HitInfo info = raycast(pos, dir, weapon->range, GROUP_BULLETS);

                int dmg = weapon->damage;
                if (dot(info.normal, dir) < -0.99f) {
                    dmg *= 2;
                }
                damage(components, grid, info.entity, info.position, dir, dmg, parent);

                particle->angle = angle;
                if (info.entity) {
                    particle->max_time = 0.9f * dist(pos, info.position) / particle->speed;
                } else {
                    particle->max_time = weapon->range / particle->speed;
                }
                add_particles(entity, 1);
            }

            weapon->recoil = fminf(weapon->max_recoil, weapon->recoil + weapon->recoil_up);
            if (PlayerComponent_get(parent)) {
                alert_enemies(components, grid, parent, weapon->sound_range);
            }

            LightComponent* light = LightComponent_get(entity);
            if (light) {
                light->brightness = light->max_brightness;
            }

            shake_camera(0.025f * weapon->shots * weapon->damage);
        }
    }

    SoundComponent* sound = SoundComponent_get(entity);
    if (sound) {
        add_sound(components, entity, weapon->sound, 1.0f, 1.0f);
    }

    if (weapon->magazine == 0) {
        reload(components, entity);
    }
}


void update_weapons(ComponentData* components, float time_step) {
    for (int i = 0; i < components->entities; i++) {
        WeaponComponent* weapon = WeaponComponent_get(i);
        if (!weapon) continue;

        weapon->cooldown = fmaxf(0.0f, weapon->cooldown - time_step);

        CoordinateComponent* coord = CoordinateComponent_get(i);
        int parent = coord->parent;
        if (parent != -1) {
            PhysicsComponent* phys = PhysicsComponent_get(parent);
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
    int i = create_entity();

    CoordinateComponent_add(i, position, 0.0f);
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(i, "pistol", 1.0, 1.0, LAYER_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    WeaponComponent_add(i, 10.0f, 20, 1, 0.0f, 12, 0.1f, 25.0f, 2.0f, AMMO_PISTOL, "pistol");
    ParticleComponent_add_type(i, PARTICLE_BULLET, 0.15f);
    ItemComponent_add(i, 1, 100, "Glock");
    SoundComponent_add(i, "metal");
    LightComponent_add(i, 2.0, 2.0 * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 10.0f)->enabled = false;

    return i;
}


int create_shotgun(ComponentData* components, sfVector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, 0.0f);
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(i, "combat_shotgun", 3.0, 1.0, LAYER_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    WeaponComponent_add(i, 0.8f, 10, 8, 0.05f * M_PI, 6, 0.25f, 25.0f, 3.0f, AMMO_SHOTGUN, "shotgun");
    ParticleComponent_add_type(i, PARTICLE_BULLET, 0.1f);
    ItemComponent_add(i, 0, 2000, "M870");
    SoundComponent_add(i, "metal");
    LightComponent_add(i, 3.0f, 2.0 * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 5.0f)->enabled = false;

    return i;
}


int create_sawed_off(ComponentData* components, sfVector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, 0.0f);
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(i, "shotgun", 2.0, 1.0, LAYER_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    WeaponComponent_add(i, 10.0f, 10, 10, 0.1f * M_PI, 2, 0.25f, 20.0f, 1.5f, AMMO_SHOTGUN, "shotgun");
    ParticleComponent_add_type(i, PARTICLE_BULLET, 0.1f);
    ItemComponent_add(i, 0, 500, "Sawed-off");
    SoundComponent_add(i, "metal");
    LightComponent_add(i, 3.0f, 2.0 * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 5.0f)->enabled = false;

    return i;
}


int create_rifle(ComponentData* components, sfVector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, 0.0f);
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(i, "assault_rifle", 3.0f, 1.0f, LAYER_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    WeaponComponent_add(i, 0.5f, 10, 1, 0.0f, -1, 0.05f, 30.0f, 3.0f, AMMO_RIFLE, "assault_rifle")->automatic = true;
    ParticleComponent_add_type(i, PARTICLE_BULLET, 0.15f);
    ItemComponent_add(i, 1, 0, "");
    SoundComponent_add(i, "metal");
    LightComponent_add(i, 4.0f, 2.0f * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 10.0f)->enabled = false;

    return i;
}


int create_assault_rifle(ComponentData* components, sfVector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, 0.0f);
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(i, "assault_rifle", 3.0f, 1.0f, LAYER_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    WeaponComponent_add(i, 10.0f, 40, 1, 0.0f, 30, 0.05f, 30.0f, 3.0f, AMMO_RIFLE, "assault_rifle")->automatic = true;
    ParticleComponent_add_type(i, PARTICLE_BULLET, 0.15f);
    ItemComponent_add(i, 1, 5000, "RK-62");
    SoundComponent_add(i, "metal");
    LightComponent_add(i, 4.0f, 2.0f * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 10.0f)->enabled = false;

    return i;
}


int create_smg(ComponentData* components, sfVector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, 0.0f);
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(i, "smg", 2.0f, 1.0f, LAYER_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    WeaponComponent_add(i, 15.0f, 20, 1, 0.0f, 20, 0.15f, 30.0f, 2.0f, AMMO_PISTOL, "pistol")->automatic = true;
    ParticleComponent_add_type(i, PARTICLE_BULLET, 0.15f);
    ItemComponent_add(i, 1, 500, "Jatimatic");
    SoundComponent_add(i, "metal");
    LightComponent_add(i, 3.0f, 2.0f * M_PI, get_color(1.0, 1.0, 1.0, 1.0), 1.0, 10.0f)->enabled = false;

    return i;
}


int create_axe(ComponentData* components, sfVector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, rand_angle());
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(i, "axe", 2.0, 1.0, LAYER_ITEMS);
    PhysicsComponent_add(i, 1.0f);
    WeaponComponent_add(i, 2.0f, 50, 1, 0.35f * M_PI, -1, 0.0f, 2.0f, 0.0f, AMMO_MELEE, "axe");
    ItemComponent_add(i, 0, 0, "Axe");
    SoundComponent_add(i, "metal");

    return i;
}


int create_sword(ComponentData* components, sfVector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, 0.0f);
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(i, "sword", 3.0, 1.0, LAYER_ITEMS);
    PhysicsComponent_add(i, 1.0f);
    WeaponComponent_add(i, 3.0f, 100, 5, 1.0f * M_PI, -1, 0.0f, 2.0f, 0.0f, AMMO_MELEE, "axe");
    ItemComponent_add(i, 0, 10000, "Fiskalibur");
    SoundComponent_add(i, "metal");

    return i;
}
 

int create_rope_gun(ComponentData* components, sfVector2f position) {
    int i = create_entity();

    CoordinateComponent_add(i, position, rand_angle());
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    ImageComponent_add(i, "pistol", 1.0, 1.0, LAYER_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    WeaponComponent_add(i, 0.5f, 0, 1, 0.0f, -1, 0.0f, 25.0f, 2.0f, AMMO_ROPE, "pistol");
    ItemComponent_add(i, 0, 0, "");
    SoundComponent_add(i, "metal");

    return i;
}


int create_lasersight(ComponentData* components, sfVector2f pos) {
    int i = create_entity();
    CoordinateComponent_add(i, pos, rand_angle());
    PhysicsComponent_add(i, 0.5f);
    ImageComponent_add(i, "flashlight", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent_add(i, 0, 500, "Laser sight");
    LightComponent_add(i, 20.0, 0.01, sfRed, 1.0, 10.0)->enabled = false;

    return i;
}


int create_ammo(ComponentData* components, sfVector2f position, AmmoType type) {
    int i = create_entity();

    CoordinateComponent_add(i, position, rand_angle());
    ColliderComponent_add_circle(i, 0.25f, GROUP_ITEMS);
    switch (type) {
        case AMMO_PISTOL:
            ImageComponent_add(i, "ammo_pistol", 1.0, 1.0, LAYER_ITEMS);
            break;
        case AMMO_RIFLE:
            ImageComponent_add(i, "ammo_rifle", 1.0, 1.0, LAYER_ITEMS);
            break;
        case AMMO_SHOTGUN:
            ImageComponent_add(i, "ammo_shotgun", 1.0, 1.0, LAYER_ITEMS);
            break;
        default:
            break;
    }
    AmmoComponent_add(i, type);
    PhysicsComponent_add(i, 0.5f);
    ItemComponent_add(i, 0, 0, "");
    SoundComponent_add(i, "metal");
    return i;
}
