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
#include "particle.h"


int get_akimbo(int entity) {
    ItemComponent* item = ItemComponent_get(entity);

    if (item && item->size > 0) {
        if (item->attachments[0] != -1 && WeaponComponent_get(item->attachments[0])) {
            return 1;
        }
    }

    return 0;
}


float get_spread(int entity) {
    WeaponComponent* weapon = WeaponComponent_get(entity);
    ItemComponent* item = ItemComponent_get(entity);

    float spread = weapon->spread;

    if (item) {
        for (int i = 0; i < item->size; i++) {
            ItemComponent* atch = ItemComponent_get(item->attachments[i]);
            if (!atch) continue;
            if (atch->type == ITEM_LASER) {
                spread *= 0.75f;
            } else if (atch->type == ITEM_SCOPE) {
                spread *= 0.5f;
            }
        }
    }

    return spread;
}


float get_recoil(int entity) {
    WeaponComponent* weapon = WeaponComponent_get(entity);
    ItemComponent* item = ItemComponent_get(entity);

    float recoil = weapon->recoil_up;

    if (item) {
        for (int i = 0; i < item->size; i++) {
            ItemComponent* atch = ItemComponent_get(item->attachments[i]);
            if (!atch) continue;
            if (atch->type == ITEM_LASER) {
                recoil *= 0.75f;
            } else if (atch->type == ITEM_SCOPE) {
                recoil *= 0.5f;
            }
        }
    }

    return recoil;
}


int get_damage(int entity) {
    WeaponComponent* weapon = WeaponComponent_get(entity);
    ItemComponent* item = ItemComponent_get(entity);

    int damage = weapon->damage;

    if (item) {
        for (int i = 0; i < item->size; i++) {
            ItemComponent* atch = ItemComponent_get(item->attachments[i]);
            if (!atch) continue;
            if (atch->type == ITEM_SILENCER) {
                damage *= 0.75f;
            } else if (atch->type == ITEM_HOLLOW_POINT) {
                damage *= 1.25f;
            }
        }
    }

    return damage;
}


float get_reload_time(int entity) {
    WeaponComponent* weapon = WeaponComponent_get(entity);
    ItemComponent* item = ItemComponent_get(entity);

    float reload_time = weapon->reload_time;

    if (item) {
        for (int i = 0; i < item->size; i++) {
            ItemComponent* atch = ItemComponent_get(item->attachments[i]);
            if (atch && atch->type == ITEM_MAGAZINE) {
                reload_time *= 0.75f;
            }
        }
    }

    return reload_time;
}


float get_fire_rate(int entity) {
    WeaponComponent* weapon = WeaponComponent_get(entity);
    ItemComponent* item = ItemComponent_get(entity);

    float fire_rate = weapon->fire_rate;

    if (item) {
        for (int i = 0; i < item->size; i++) {
            ItemComponent* atch = ItemComponent_get(item->attachments[i]);
            if (!atch) continue;
            if (atch->type == ITEM_SCOPE) {
                fire_rate *= 0.75f;
            }
        }
    }

    return fire_rate;
}


int get_magazine(int entity) {
    WeaponComponent* weapon = WeaponComponent_get(entity);
    ItemComponent* item = ItemComponent_get(entity);

    int magazine = weapon->max_magazine;
    for (int i = 0; i < item->size; i++) {
        ItemComponent* atch = ItemComponent_get(item->attachments[i]);
        if (!atch) continue;
        if (atch->type == ITEM_MAGAZINE) {
            magazine = floorf(magazine * 1.5f);
        }
    }

    return magazine;
}


float has_silencer(int entity) {
    ItemComponent* item = ItemComponent_get(entity);
    for (int i = 0; i < item->size; i++) {
        ItemComponent* atch = ItemComponent_get(item->attachments[i]);
        if (atch && atch->type == ITEM_SILENCER) {
            return true;
        }
    }

    return false;
}


void reload(int i) {
    WeaponComponent* weapon = WeaponComponent_get(i);
    if (weapon->max_magazine == -1) {
        return;
    }

    PlayerComponent* player = PlayerComponent_get(CoordinateComponent_get(i)->parent);
    AmmoComponent* ammo = AmmoComponent_get(player->ammo[weapon->ammo_type]);
    if (!ammo || ammo->size == 0) {
        return;
    }

    int akimbo = get_akimbo(i);

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


void create_energy(Vector2f position, Vector2f velocity) {
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


void update_energy() {
    for (int i = 0; i < game_data->components->entities; i++) {
        ColliderComponent* col = ColliderComponent_get(i);
        if (!col) continue;
        if (col->group == GROUP_ENERGY) {
            PhysicsComponent* phys = PhysicsComponent_get(i);
            LightComponent* light = LightComponent_get(i);

            for (ListNode* node = phys->collision.entities->head; node; node = node->next) {
                int j = node->value;
                damage(j, get_position(i), zeros(), 20, -1);
            }
            
            if (phys->collision.entities->size > 0) {
                light->brightness = 0.5f;
                add_particles(i, 10);
                clear_grid(i);
                ColliderComponent_get(i)->enabled = false;
                ImageComponent_remove(i);
                add_sound(i, "energy", 0.5f, randf(1.2f, 1.5f));
                phys->lifetime = 1.0f;
            }
        }
    }
}


int create_rope(Vector2f start, Vector2f end) {
    Vector2f r = diff(end, start);
    float seg_len = 0.5f;
    float len = norm(r);

    int prev = -1;
    int n = len / seg_len;
    for (int i = 0; i < n; i++) {
        int current = create_entity();
        Vector2f pos = sum(start, mult((i * seg_len) / len, r));
        CoordinateComponent_add(current, pos, 0.0f);
        PhysicsComponent_add(current, 0.2f);
        JointComponent_add(current, prev, 0.0f, seg_len, 1.0f);
        ColliderComponent_add_circle(current, 0.25f, GROUP_BULLETS);
        ImageComponent_add(current, "rope", 0.0f, 0.0f, LAYER_ITEMS);

        prev = current;
    }

    return prev;
}


int rope_root(int rope) {
    int j = rope;
    while (true) {
        int p = JointComponent_get(j)->parent;
        if (p == -1) break;
        j = p;
    }

    return j;
}


int create_flame(Vector2f position, Vector2f velocity) {
    int i = create_entity();

    CoordinateComponent* coord = CoordinateComponent_add(i, position, 0.0);
    coord->lifetime = 0.5f;
    PhysicsComponent* phys = PhysicsComponent_add(i, 0.0f);
    phys->velocity = velocity;
    phys->drag = 0.0f;
    phys->max_speed = norm(velocity);
    phys->bounce = 0.0f;
    Color orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent* light = LightComponent_add(i, 2.0f, 2.0 * M_PI, COLOR_ORANGE, 0.8f, 10.0f);
    light->flicker = 0.1f;
    light->rays = 10;
    ParticleComponent* particle = ParticleComponent_add_type(i, PARTICLE_FIRE, 0.5f);
    particle->rate = 40.0f;
    particle->speed = 0.0f;
    ColliderComponent* collider = ColliderComponent_add_circle(i, 0.35, GROUP_BULLETS);
    collider->trigger_type = TRIGGER_DAMAGE;
    SoundComponent* sound = SoundComponent_add(i, "");
    strcpy(sound->loop_sound, "fire");
    ImageComponent_add(i, "", 0.0f, 0.0f, LAYER_PARTICLES);

    return i;
}


void attack(int entity) {
    WeaponComponent* weapon = WeaponComponent_get(entity);
    int parent = CoordinateComponent_get(entity)->parent;

    if (weapon->magazine == 0) {
        reload(entity);
        return;
    }

    if (weapon->cooldown != 0.0f) {
        return;
    }

    int akimbo = get_akimbo(entity);
    weapon->cooldown = 1.0f / ((1 + akimbo) * get_fire_rate(entity));
    if (weapon->max_magazine != -1) {
        weapon->magazine--;
    }

    PlayerComponent* player = PlayerComponent_get(parent);
    if (player) {
        AnimationComponent_get(player->arms)->framerate = 20.0f;
    }

    Vector2f pos = get_position(parent);

    switch (weapon->ammo_type) {
        case AMMO_MELEE: {
            HitInfo min_info[7];
            float min_dist[7];
            for (int i = 0; i < weapon->shots; i++) {
                min_info[i].entity = -1;
                min_dist[i] = INFINITY;
            }

            int rays = 7;
            for (int i = 0; i < rays; i++) {
                float angle = i * weapon->spread / (rays - 1) - 0.5f * weapon->spread;
                Vector2f dir = polar_to_cartesian(1.0, get_angle(parent) + angle);
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
                Vector2f dir = normalized(diff(min_info[i].position, pos));
                damage(min_info[i].entity, min_info[i].position, dir, dmg, parent);
                shake_camera(0.0125f * weapon->damage);
            }
            break;
        } case AMMO_ENERGY: {
            for (int i = 0; i < weapon->shots; i++) {
                float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
                if (weapon->spread > 0.0f) {
                    angle = i * weapon->spread / (weapon->shots - 1) - 0.5f * weapon->spread + randf(-0.1f, 0.1f);
                }
                Vector2f vel = polar_to_cartesian(7.0f, get_angle(parent) + angle);
                create_energy(sum(get_position(entity), mult(1.0f / 14.0f, vel)), vel);
            }
            break;
        } case AMMO_ROPE: {
            float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
            Vector2f dir = polar_to_cartesian(1.0f, get_angle(parent) + angle);
            HitInfo info = raycast(pos, dir, weapon->range, GROUP_BULLETS);

            int i = create_rope(info.position, get_position(parent));

            int j = rope_root(i);
            JointComponent_get(j)->parent = info.entity;
            // CoordinateComponent* coord = CoordinateComponent_get(j);
            // coord->parent = j;
            // coord->
            // PhysicsComponent_remove(j);

            damage(info.entity, info.position, dir, weapon->damage, parent);

            break;
        } case AMMO_FLAME: {
            create_flame(get_position(entity), polar_to_cartesian(20.0f, get_angle(parent)));
            break;
        } default: {
            ParticleComponent* particle = ParticleComponent_get(entity);

            for (int i = 0; i < weapon->shots; i++) {
                float angle = randf(-0.5f * weapon->recoil, 0.5f * weapon->recoil);
                float spread = get_spread(entity);
                if (spread > 0.0f) {
                    angle = i * spread / (weapon->shots - 1) - 0.5f * spread + randf(-0.1f, 0.1f);
                }
                Vector2f dir = polar_to_cartesian(1.0, get_angle(parent) + angle);

                HitInfo info;
                Vector2f start = pos;
                float range = weapon->range;
                for (int p = 0; p <= weapon->penetration; p++) {
                    info = raycast(start, dir, range, GROUP_BULLETS);

                    int dmg = get_damage(entity);
                    if (dot(info.normal, dir) < -0.99f) {
                        dmg *= 2;
                    }
                    damage(info.entity, info.position, dir, dmg, parent);

                    start = sum(info.position, mult(0.1f, dir));
                    range -= dist(pos, info.position);

                    if (!HealthComponent_get(info.entity)) {
                        break;
                    }
                }

                particle->angle = angle;
                if (info.entity != -1) {
                    particle->max_time = 0.5f * dist(pos, info.position) / particle->speed;
                } else {
                    particle->max_time = weapon->range / particle->speed;
                }
                add_particles(entity, 1);
            }

            weapon->recoil = fminf(weapon->max_recoil, weapon->recoil + weapon->recoil_up);
            if (PlayerComponent_get(parent) && !has_silencer(entity)) {
                alert_enemies(parent, weapon->sound_range);
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
        if (has_silencer(entity)) {
            add_sound(entity, weapon->sound, 0.5f, 1.0f);
            add_sound(entity, "silencer", 0.5f, 1.0f);
        } else {
            add_sound(entity, weapon->sound, 1.0f, 1.0f);
        }
    }

    if (weapon->magazine == 0) {
        reload(entity);
    }
}


void update_weapons(float time_step) {
    for (int i = 0; i < game_data->components->entities; i++) {
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
                float x_max = 1.0f / get_fire_rate(i);
                float x = 2.0f * (x_max - weapon->cooldown) / x_max;
                coord->angle = M_PI * powf(x - 1.0f, 2.0f) - 0.45f * M_PI;
            }
        }
    }
}


int create_pistol(Vector2f position) {
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


int create_shotgun(Vector2f position) {
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


int create_sawed_off(Vector2f position) {
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


int create_rifle(Vector2f position) {
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


int create_assault_rifle(Vector2f position) {
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


int create_smg(Vector2f position) {
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


int create_axe(Vector2f position) {
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


int create_sword(Vector2f position) {
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
 

int create_rope_gun(Vector2f position) {
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


int create_lasersight(Vector2f pos) {
    int i = create_entity();
    CoordinateComponent_add(i, pos, rand_angle());
    PhysicsComponent_add(i, 0.5f);
    ImageComponent_add(i, "flashlight", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent_add(i, 0, 500, "Laser sight");
    LightComponent_add(i, 20.0, 0.01, COLOR_RED, 1.0, 10.0)->enabled = false;

    return i;
}


int create_ammo(Vector2f position, AmmoType type) {
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
