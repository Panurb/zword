#include <string.h>
#include <math.h>

#include "serialize_binary.h"
#include "component.h"
#include "particle.h"
#include "image.h"
#include "util.h"


// --- Helper: write/read length-prefixed strings ---

static int write_string(uint8_t* buf, int remaining, const char* str) {
    uint8_t len = (uint8_t)strlen(str);
    int need = 1 + (int)len;
    if (need > remaining) return 0;
    buf[0] = len;
    memcpy(buf + 1, str, len);
    return need;
}

static int read_string(const uint8_t* buf, int remaining, char* out, int out_size) {
    if (remaining < 1) return 0;
    uint8_t len = buf[0];
    if (1 + (int)len > remaining) return 0;
    int copy = (len < out_size - 1) ? len : out_size - 1;
    memcpy(out, buf + 1, copy);
    out[copy] = '\0';
    return 1 + (int)len;
}

// --- Helper: write/read primitives ---

static int write_f32(uint8_t* buf, int remaining, float v) {
    if (remaining < 4) return 0;
    memcpy(buf, &v, 4);
    return 4;
}

static int write_i32(uint8_t* buf, int remaining, int32_t v) {
    if (remaining < 4) return 0;
    memcpy(buf, &v, 4);
    return 4;
}

static int write_i16(uint8_t* buf, int remaining, int16_t v) {
    if (remaining < 2) return 0;
    memcpy(buf, &v, 2);
    return 2;
}

static int write_u8(uint8_t* buf, int remaining, uint8_t v) {
    if (remaining < 1) return 0;
    buf[0] = v;
    return 1;
}

static float read_f32(const uint8_t** p) {
    float v;
    memcpy(&v, *p, 4);
    *p += 4;
    return v;
}

static int32_t read_i32(const uint8_t** p) {
    int32_t v;
    memcpy(&v, *p, 4);
    *p += 4;
    return v;
}

static int16_t read_i16(const uint8_t** p) {
    int16_t v;
    memcpy(&v, *p, 2);
    *p += 2;
    return v;
}

static uint8_t read_u8(const uint8_t** p) {
    uint8_t v = **p;
    *p += 1;
    return v;
}


// ============================================================
// Serialize
// ============================================================

int binary_serialize_entity(uint8_t* buf, int buf_size, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (!coord) return 0;

    // Determine which optional components this entity has
    uint32_t flags = 0;

    ImageComponent*     image    = ImageComponent_get(entity);
    PhysicsComponent*   physics  = PhysicsComponent_get(entity);
    ColliderComponent*  collider = ColliderComponent_get(entity);
    PlayerComponent*    player   = PlayerComponent_get(entity);
    EnemyComponent*     enemy    = EnemyComponent_get(entity);
    HealthComponent*    health   = HealthComponent_get(entity);
    WeaponComponent*    weapon   = WeaponComponent_get(entity);
    ItemComponent*      item     = ItemComponent_get(entity);
    WaypointComponent*  waypoint = WaypointComponent_get(entity);
    ParticleComponent*  particle = ParticleComponent_get(entity);
    LightComponent*     light    = LightComponent_get(entity);
    SoundComponent*     sound    = SoundComponent_get(entity);
    AmmoComponent*      ammo     = AmmoComponent_get(entity);
    AnimationComponent* anim     = AnimationComponent_get(entity);
    DoorComponent*      door     = DoorComponent_get(entity);
    JointComponent*     joint    = JointComponent_get(entity);
    VehicleComponent*   vehicle  = VehicleComponent_get(entity);
    TextComponent*      text     = TextComponent_get(entity);

    if (image)    flags |= BFLAG_HAS_IMAGE;
    if (physics)  flags |= BFLAG_HAS_PHYSICS;
    if (collider) flags |= BFLAG_HAS_COLLIDER;
    if (player)   flags |= BFLAG_HAS_PLAYER;
    if (enemy)    flags |= BFLAG_HAS_ENEMY;
    if (health)   flags |= BFLAG_HAS_HEALTH;
    if (weapon)   flags |= BFLAG_HAS_WEAPON;
    if (item)     flags |= BFLAG_HAS_ITEM;
    if (waypoint) flags |= BFLAG_HAS_WAYPOINT;
    if (particle) flags |= BFLAG_HAS_PARTICLE;
    if (light)    flags |= BFLAG_HAS_LIGHT;
    if (sound)    flags |= BFLAG_HAS_SOUND;
    if (ammo)     flags |= BFLAG_HAS_AMMO;
    if (anim)     flags |= BFLAG_HAS_ANIMATION;
    if (door)     flags |= BFLAG_HAS_DOOR;
    if (joint)    flags |= BFLAG_HAS_JOINT;
    if (vehicle)  flags |= BFLAG_HAS_VEHICLE;
    if (text)     flags |= BFLAG_HAS_TEXT;

    uint8_t* ptr = buf;
    int rem = buf_size;
    int n;

    // --- Coordinate (always first, unconditional) ---
    // pos_x(f32) pos_y(f32) angle(f32) parent(i16) scale_x(f32) scale_y(f32) lifetime(f32) = 30 bytes
    if (rem < 30) return 0;
    n = write_f32(ptr, rem, coord->position.x);
    ptr += n; rem -= n;
    n = write_f32(ptr, rem, coord->position.y);
    ptr += n; rem -= n;
    n = write_f32(ptr, rem, coord->angle);
    ptr += n; rem -= n;
    n = write_i16(ptr, rem, (int16_t)coord->parent);
    ptr += n; rem -= n;
    n = write_f32(ptr, rem, coord->scale.x);
    ptr += n; rem -= n;
    n = write_f32(ptr, rem, coord->scale.y);
    ptr += n; rem -= n;
    n = write_f32(ptr, rem, coord->lifetime);
    ptr += n; rem -= n;

    // Write flags (4 bytes)
    if (rem < 4) return 0;
    memcpy(ptr, &flags, 4);
    ptr += 4; rem -= 4;

    // --- Image ---
    if (image) {
        n = write_string(ptr, rem, image->filename);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, image->width);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, image->height);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)image->layer);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, image->alpha);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, image->shine);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Physics (creation + runtime) ---
    if (physics) {
        n = write_f32(ptr, rem, physics->mass);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, physics->bounce);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, physics->max_speed);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, physics->angular_drag);
        if (!n) return 0; ptr += n; rem -= n;
        // Runtime state
        n = write_f32(ptr, rem, physics->velocity.x);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, physics->velocity.y);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Collider ---
    if (collider) {
        n = write_u8(ptr, rem, (uint8_t)collider->type);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, collider->width);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, collider->height);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)collider->group);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)collider->enabled);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)collider->trigger_type);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Player (runtime state) ---
    if (player) {
        n = write_u8(ptr, rem, (uint8_t)player->state);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i32(ptr, rem, (int32_t)player->money);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)player->item);
        if (!n) return 0; ptr += n; rem -= n;
        for (int i = 0; i < 4; i++) {
            n = write_i16(ptr, rem, (int16_t)player->inventory[i]);
            if (!n) return 0; ptr += n; rem -= n;
        }
        for (int i = 0; i < 4; i++) {
            n = write_i16(ptr, rem, (int16_t)player->ammo[i]);
            if (!n) return 0; ptr += n; rem -= n;
        }
        for (int i = 0; i < 3; i++) {
            n = write_i16(ptr, rem, (int16_t)player->keys[i]);
            if (!n) return 0; ptr += n; rem -= n;
        }
        n = write_i16(ptr, rem, (int16_t)player->arms);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)player->vehicle);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)player->target);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, player->use_timer);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)player->grabbed_item);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, player->won ? 1 : 0);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, player->money_timer);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i32(ptr, rem, (int32_t)player->money_increment);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Enemy (creation + runtime) ---
    if (enemy) {
        // Creation params
        n = write_f32(ptr, rem, enemy->fov);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, enemy->idle_speed);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, enemy->walk_speed);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, enemy->run_speed);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, enemy->attack_delay);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, enemy->turn_speed);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, enemy->attack_angle);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)enemy->spawner);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)enemy->boss);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i32(ptr, rem, (int32_t)enemy->bounty);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, enemy->vision_range);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, enemy->acceleration);
        if (!n) return 0; ptr += n; rem -= n;
        // Runtime state
        n = write_u8(ptr, rem, (uint8_t)enemy->state);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)enemy->weapon);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)enemy->target);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Health (creation + runtime) ---
    if (health) {
        n = write_i16(ptr, rem, (int16_t)health->health);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)health->max_health);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_string(ptr, rem, health->dead_image);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_string(ptr, rem, health->decal);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_string(ptr, rem, health->die_sound);
        if (!n) return 0; ptr += n; rem -= n;
        for (int i = 0; i < DAMAGE_COUNT; i++) {
            n = write_f32(ptr, rem, health->damage_factor[i]);
            if (!n) return 0; ptr += n; rem -= n;
        }
        // Runtime state
        n = write_u8(ptr, rem, health->dead ? 1 : 0);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)health->status.type);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Weapon (creation + runtime) ---
    if (weapon) {
        // Creation params
        n = write_f32(ptr, rem, weapon->fire_rate);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)weapon->damage);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)weapon->shots);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, weapon->spread);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)weapon->max_magazine);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, weapon->recoil_up);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, weapon->max_recoil);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, weapon->range);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, weapon->sound_range);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, weapon->reload_time);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)weapon->ammo_type);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_string(ptr, rem, weapon->sound);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)weapon->automatic);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)weapon->penetration);
        if (!n) return 0; ptr += n; rem -= n;
        // Runtime state
        n = write_f32(ptr, rem, weapon->recoil);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, weapon->cooldown);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)weapon->magazine);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, weapon->reloading ? 1 : 0);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Item (creation + runtime) ---
    if (item) {
        n = write_u8(ptr, rem, (uint8_t)item->size);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)item->price);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_string(ptr, rem, item->name);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)item->type);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)item->value);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, item->use_time);
        if (!n) return 0; ptr += n; rem -= n;
        // Runtime state
        for (int i = 0; i < 5; i++) {
            n = write_i16(ptr, rem, (int16_t)item->attachments[i]);
            if (!n) return 0; ptr += n; rem -= n;
        }
    }

    // --- Waypoint ---
    // No data fields

    // --- Particle ---
    if (particle) {
        n = write_u8(ptr, rem, (uint8_t)particle->type);
        if (!n) return 0; ptr += n; rem -= n;
        if (particle->type == PARTICLE_NONE) {
            // Custom particle: full params
            n = write_f32(ptr, rem, particle->angle);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_f32(ptr, rem, particle->spread);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_f32(ptr, rem, particle->start_size);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_f32(ptr, rem, particle->end_size);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_f32(ptr, rem, particle->speed);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_f32(ptr, rem, particle->rate);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_u8(ptr, rem, (uint8_t)particle->outer_color.r);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_u8(ptr, rem, (uint8_t)particle->outer_color.g);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_u8(ptr, rem, (uint8_t)particle->outer_color.b);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_u8(ptr, rem, (uint8_t)particle->inner_color.r);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_u8(ptr, rem, (uint8_t)particle->inner_color.g);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_u8(ptr, rem, (uint8_t)particle->inner_color.b);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_u8(ptr, rem, (uint8_t)particle->loop);
            if (!n) return 0; ptr += n; rem -= n;
        } else {
            // Typed particle
            n = write_f32(ptr, rem, particle->start_size);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_f32(ptr, rem, particle->width);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_f32(ptr, rem, particle->height);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_u8(ptr, rem, (uint8_t)particle->loop);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_f32(ptr, rem, particle->rate);
            if (!n) return 0; ptr += n; rem -= n;
        }
    }

    // --- Light (creation + runtime) ---
    if (light) {
        n = write_f32(ptr, rem, light->range);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, light->angle);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)light->color.r);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)light->color.g);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)light->color.b);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, light->max_brightness);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, light->speed);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)light->enabled);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, light->flicker);
        if (!n) return 0; ptr += n; rem -= n;
        // Runtime state
        n = write_f32(ptr, rem, light->brightness);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Sound ---
    if (sound) {
        n = write_string(ptr, rem, sound->hit_sound);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_string(ptr, rem, sound->loop_sound);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Ammo ---
    if (ammo) {
        n = write_u8(ptr, rem, (uint8_t)ammo->type);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)ammo->size);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Animation (creation + runtime) ---
    if (anim) {
        n = write_i16(ptr, rem, (int16_t)anim->frames);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, anim->framerate);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)anim->play_once);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, anim->wind_factor);
        if (!n) return 0; ptr += n; rem -= n;
        // Runtime state
        n = write_i16(ptr, rem, (int16_t)anim->current_frame);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Door ---
    if (door) {
        n = write_i16(ptr, rem, (int16_t)door->price);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)door->locked);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)door->key);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Joint ---
    if (joint) {
        n = write_i16(ptr, rem, (int16_t)joint->parent);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, joint->min_length);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, joint->max_length);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, joint->strength);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, joint->max_angle);
        if (!n) return 0; ptr += n; rem -= n;
    }

    // --- Vehicle (creation + runtime) ---
    if (vehicle) {
        // Creation params
        n = write_f32(ptr, rem, vehicle->max_fuel);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)vehicle->size);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, vehicle->acceleration);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, vehicle->max_speed);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_f32(ptr, rem, vehicle->turning);
        if (!n) return 0; ptr += n; rem -= n;
        for (int i = 0; i < vehicle->size && i < 4; i++) {
            n = write_f32(ptr, rem, vehicle->seats[i].x);
            if (!n) return 0; ptr += n; rem -= n;
            n = write_f32(ptr, rem, vehicle->seats[i].y);
            if (!n) return 0; ptr += n; rem -= n;
        }
        // Runtime state
        n = write_f32(ptr, rem, vehicle->fuel);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, vehicle->on_road ? 1 : 0);
        if (!n) return 0; ptr += n; rem -= n;
        for (int i = 0; i < 4; i++) {
            n = write_i16(ptr, rem, (int16_t)vehicle->riders[i]);
            if (!n) return 0; ptr += n; rem -= n;
        }
    }

    // --- Text ---
    if (text) {
        n = write_string(ptr, rem, text->source_string);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_i16(ptr, rem, (int16_t)text->size);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)text->color.r);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)text->color.g);
        if (!n) return 0; ptr += n; rem -= n;
        n = write_u8(ptr, rem, (uint8_t)text->color.b);
        if (!n) return 0; ptr += n; rem -= n;
    }

    return (int)(ptr - buf);
}


// ============================================================
// Deserialize (get-or-add pattern: update in place if exists, create if not)
// ============================================================

int binary_deserialize_entity(const uint8_t* buf, int buf_size, int entity, bool smooth) {
    const uint8_t* ptr = buf;
    const uint8_t* end = buf + buf_size;

#define CHECK(need) do { if (ptr + (need) > end) return 0; } while(0)

    // --- Coordinate (always present) ---
    // pos_x(f32) pos_y(f32) angle(f32) parent(i16) scale_x(f32) scale_y(f32) lifetime(f32)
    CHECK(30);
    float pos_x = read_f32(&ptr);
    float pos_y = read_f32(&ptr);
    float angle = read_f32(&ptr);
    int16_t parent = read_i16(&ptr);
    float scale_x = read_f32(&ptr);
    float scale_y = read_f32(&ptr);
    float lifetime = read_f32(&ptr);

    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (coord) {
        if (smooth) {
            float t = 0.3f;
            coord->position.x = lerp(coord->position.x, pos_x, t);
            coord->position.y = lerp(coord->position.y, pos_y, t);
            coord->angle = lerp_angle(coord->angle, angle, t);
        } else {
            coord->position.x = pos_x;
            coord->position.y = pos_y;
            coord->angle = angle;
        }
        coord->parent = (int)parent;  // raw host ID; remapped by netgame.c post-pass
        coord->scale.x = scale_x;
        coord->scale.y = scale_y;
        coord->lifetime = lifetime;
    }
    // If coord is NULL the caller made an error (entity must exist), but don't crash

    // --- Flags ---
    CHECK(4);
    uint32_t flags;
    memcpy(&flags, ptr, 4);
    ptr += 4;

    // --- Image ---
    if (flags & BFLAG_HAS_IMAGE) {
        Filename filename;
        int sn = read_string(ptr, (int)(end - ptr), filename, sizeof(filename));
        if (!sn) return 0;
        ptr += sn;
        CHECK(4 + 4 + 1 + 4 + 4);
        float width = read_f32(&ptr);
        float height = read_f32(&ptr);
        uint8_t layer = read_u8(&ptr);
        float alpha = read_f32(&ptr);
        float shine = read_f32(&ptr);

        ImageComponent* image = ImageComponent_get(entity);
        if (image) {
            // Update existing: check if texture changed
            if (strcmp(image->filename, filename) != 0) {
                change_texture(entity, filename, width, height);
            }
            image->width = width;
            image->height = height;
            if (image->layer != (Layer)layer) {
                change_layer(entity, (Layer)layer);
            }
            image->alpha = alpha;
            image->shine = shine;
        } else {
            image = ImageComponent_add(entity, filename, width, height, (Layer)layer);
            image->alpha = alpha;
            image->shine = shine;
        }
    }

    // --- Physics ---
    if (flags & BFLAG_HAS_PHYSICS) {
        CHECK(4 + 4 + 4 + 4 + 4 + 4);
        float mass = read_f32(&ptr);
        float bounce = read_f32(&ptr);
        float max_speed = read_f32(&ptr);
        float angular_drag = read_f32(&ptr);
        float vel_x = read_f32(&ptr);
        float vel_y = read_f32(&ptr);

        PhysicsComponent* physics = PhysicsComponent_get(entity);
        if (!physics) {
            physics = PhysicsComponent_add(entity, mass);
        }
        physics->mass = mass;
        physics->bounce = bounce;
        physics->max_speed = max_speed;
        physics->angular_drag = angular_drag;
        physics->velocity.x = vel_x;
        physics->velocity.y = vel_y;
    }

    // --- Collider ---
    if (flags & BFLAG_HAS_COLLIDER) {
        CHECK(1 + 4 + 4 + 1 + 1 + 1);
        uint8_t type = read_u8(&ptr);
        float width = read_f32(&ptr);
        float height = read_f32(&ptr);
        uint8_t group = read_u8(&ptr);
        uint8_t enabled = read_u8(&ptr);
        uint8_t trigger_type = read_u8(&ptr);

        ColliderComponent* collider = ColliderComponent_get(entity);
        if (!collider) {
            if (type == COLLIDER_CIRCLE) {
                collider = ColliderComponent_add_circle(entity, 0.5f * width, (ColliderGroup)group);
            } else {
                collider = ColliderComponent_add_rectangle(entity, width, height, (ColliderGroup)group);
            }
        }
        collider->width = width;
        collider->height = height;
        collider->enabled = enabled;
        collider->trigger_type = (TriggerType)trigger_type;
    }

    // --- Player ---
    if (flags & BFLAG_HAS_PLAYER) {
        CHECK(1 + 4 + 1 + 4*2 + 4*2 + 3*2 + 2 + 2 + 2 + 4 + 2 + 1 + 4 + 4);
        uint8_t state = read_u8(&ptr);
        int32_t money = read_i32(&ptr);
        uint8_t item_slot = read_u8(&ptr);
        int16_t inventory[4];
        for (int i = 0; i < 4; i++) inventory[i] = read_i16(&ptr);
        int16_t ammo_slots[4];
        for (int i = 0; i < 4; i++) ammo_slots[i] = read_i16(&ptr);
        int16_t keys[3];
        for (int i = 0; i < 3; i++) keys[i] = read_i16(&ptr);
        int16_t arms = read_i16(&ptr);
        int16_t vehicle_id = read_i16(&ptr);
        int16_t target = read_i16(&ptr);
        float use_timer = read_f32(&ptr);
        int16_t grabbed_item = read_i16(&ptr);
        uint8_t won = read_u8(&ptr);
        float money_timer = read_f32(&ptr);
        int32_t money_increment = read_i32(&ptr);

        PlayerComponent* player = PlayerComponent_get(entity);
        if (!player) {
            player = PlayerComponent_add(entity);
        }
        player->state = (PlayerState)state;
        player->money = (int)money;
        player->item = (int)item_slot;
        for (int i = 0; i < 4; i++) player->inventory[i] = (int)inventory[i];
        for (int i = 0; i < 4; i++) player->ammo[i] = (int)ammo_slots[i];
        for (int i = 0; i < 3; i++) player->keys[i] = (int)keys[i];
        player->arms = (int)arms;              // raw host ID; remapped by post-pass
        player->vehicle = (int)vehicle_id;     // raw host ID; remapped by post-pass
        player->target = (int)target;          // raw host ID; remapped by post-pass
        player->use_timer = use_timer;
        player->grabbed_item = (int)grabbed_item;  // raw host ID; remapped by post-pass
        player->won = won != 0;
        player->money_timer = money_timer;
        player->money_increment = (int)money_increment;
    }

    // --- Enemy ---
    if (flags & BFLAG_HAS_ENEMY) {
        CHECK(4*7 + 1 + 1 + 4 + 4 + 4 + 1 + 2 + 2);
        float fov = read_f32(&ptr);
        float idle_speed = read_f32(&ptr);
        float walk_speed = read_f32(&ptr);
        float run_speed = read_f32(&ptr);
        float attack_delay = read_f32(&ptr);
        float turn_speed = read_f32(&ptr);
        float attack_angle = read_f32(&ptr);
        uint8_t spawner = read_u8(&ptr);
        uint8_t boss = read_u8(&ptr);
        int32_t bounty = read_i32(&ptr);
        float vision_range = read_f32(&ptr);
        float acceleration = read_f32(&ptr);
        // Runtime state
        uint8_t state = read_u8(&ptr);
        int16_t weapon_id = read_i16(&ptr);
        int16_t target = read_i16(&ptr);

        EnemyComponent* enemy = EnemyComponent_get(entity);
        if (!enemy) {
            enemy = EnemyComponent_add(entity);
        }
        enemy->fov = fov;
        enemy->idle_speed = idle_speed;
        enemy->walk_speed = walk_speed;
        enemy->run_speed = run_speed;
        enemy->attack_delay = attack_delay;
        enemy->attack_timer = attack_delay;
        enemy->turn_speed = turn_speed;
        enemy->attack_angle = attack_angle;
        enemy->spawner = spawner;
        enemy->boss = boss;
        enemy->bounty = (int)bounty;
        enemy->vision_range = vision_range;
        enemy->acceleration = acceleration;
        enemy->state = (EnemyState)state;
        enemy->weapon = (int)weapon_id;    // raw host ID; remapped by post-pass
        enemy->target = (int)target;       // raw host ID; remapped by post-pass
    }

    // --- Health ---
    if (flags & BFLAG_HAS_HEALTH) {
        CHECK(2 + 2);
        int16_t hp = read_i16(&ptr);
        int16_t max_hp = read_i16(&ptr);
        Filename dead_image, decal, die_sound;
        int sn;
        sn = read_string(ptr, (int)(end - ptr), dead_image, sizeof(dead_image));
        if (!sn) return 0; ptr += sn;
        sn = read_string(ptr, (int)(end - ptr), decal, sizeof(decal));
        if (!sn) return 0; ptr += sn;
        sn = read_string(ptr, (int)(end - ptr), die_sound, sizeof(die_sound));
        if (!sn) return 0; ptr += sn;
        CHECK(4 * DAMAGE_COUNT + 1 + 1);
        float damage_factor[DAMAGE_COUNT];
        for (int i = 0; i < DAMAGE_COUNT; i++) {
            damage_factor[i] = read_f32(&ptr);
        }
        uint8_t dead = read_u8(&ptr);
        uint8_t status_type = read_u8(&ptr);

        HealthComponent* health = HealthComponent_get(entity);
        if (!health) {
            health = HealthComponent_add(entity, (int)hp, dead_image, decal, die_sound);
        }
        health->health = (int)hp;
        health->max_health = (int)max_hp;
        strcpy(health->dead_image, dead_image);
        strcpy(health->decal, decal);
        strcpy(health->die_sound, die_sound);
        for (int i = 0; i < DAMAGE_COUNT; i++) {
            health->damage_factor[i] = damage_factor[i];
        }
        health->dead = dead != 0;
        health->status.type = (StatusEffect)status_type;
    }

    // --- Weapon ---
    if (flags & BFLAG_HAS_WEAPON) {
        CHECK(4 + 2 + 2 + 4 + 2 + 4 + 4 + 4 + 4 + 4 + 1);
        float fire_rate = read_f32(&ptr);
        int16_t damage = read_i16(&ptr);
        int16_t shots = read_i16(&ptr);
        float spread = read_f32(&ptr);
        int16_t max_magazine = read_i16(&ptr);
        float recoil_up = read_f32(&ptr);
        float max_recoil = read_f32(&ptr);
        float range = read_f32(&ptr);
        float sound_range = read_f32(&ptr);
        float reload_time = read_f32(&ptr);
        uint8_t ammo_type = read_u8(&ptr);
        Filename sound_name;
        int sn = read_string(ptr, (int)(end - ptr), sound_name, sizeof(sound_name));
        if (!sn) return 0; ptr += sn;
        CHECK(1 + 2 + 4 + 4 + 2 + 1);
        uint8_t automatic = read_u8(&ptr);
        int16_t penetration = read_i16(&ptr);
        // Runtime state
        float recoil = read_f32(&ptr);
        float cooldown = read_f32(&ptr);
        int16_t magazine = read_i16(&ptr);
        uint8_t reloading = read_u8(&ptr);

        WeaponComponent* weapon = WeaponComponent_get(entity);
        if (!weapon) {
            weapon = WeaponComponent_add(entity, fire_rate, (int)damage, (int)shots,
                spread, (int)max_magazine, recoil_up, range, reload_time, (AmmoType)ammo_type, sound_name);
        }
        weapon->fire_rate = fire_rate;
        weapon->damage = (int)damage;
        weapon->shots = (int)shots;
        weapon->spread = spread;
        weapon->max_magazine = (int)max_magazine;
        weapon->recoil_up = recoil_up;
        weapon->max_recoil = max_recoil;
        weapon->range = range;
        weapon->sound_range = sound_range;
        weapon->reload_time = reload_time;
        weapon->ammo_type = (AmmoType)ammo_type;
        strcpy(weapon->sound, sound_name);
        weapon->automatic = automatic;
        weapon->penetration = (int)penetration;
        weapon->recoil = recoil;
        weapon->cooldown = cooldown;
        weapon->magazine = (int)magazine;
        weapon->reloading = reloading != 0;
    }

    // --- Item ---
    if (flags & BFLAG_HAS_ITEM) {
        CHECK(1 + 2);
        uint8_t size = read_u8(&ptr);
        int16_t price = read_i16(&ptr);
        ButtonText name;
        int sn = read_string(ptr, (int)(end - ptr), name, sizeof(name));
        if (!sn) return 0; ptr += sn;
        CHECK(1 + 2 + 4 + 5*2);
        uint8_t type = read_u8(&ptr);
        int16_t value = read_i16(&ptr);
        float use_time = read_f32(&ptr);
        // Runtime state
        int16_t attachments[5];
        for (int i = 0; i < 5; i++) attachments[i] = read_i16(&ptr);

        ItemComponent* item_comp = ItemComponent_get(entity);
        if (!item_comp) {
            item_comp = ItemComponent_add(entity, (int)size, (int)price, name);
        }
        item_comp->size = (int)size;
        item_comp->price = (int)price;
        strcpy(item_comp->name, name);
        item_comp->type = (ItemType)type;
        item_comp->value = (int)value;
        item_comp->use_time = use_time;
        for (int i = 0; i < 5; i++) {
            item_comp->attachments[i] = (int)attachments[i];  // raw host IDs; remapped by post-pass
        }
    }

    // --- Waypoint ---
    if (flags & BFLAG_HAS_WAYPOINT) {
        WaypointComponent* wp = WaypointComponent_get(entity);
        if (!wp) {
            WaypointComponent_add(entity);
        }
    }

    // --- Particle ---
    if (flags & BFLAG_HAS_PARTICLE) {
        CHECK(1);
        uint8_t type = read_u8(&ptr);
        if (type == PARTICLE_NONE) {
            CHECK(4*6 + 6 + 1);
            float p_angle = read_f32(&ptr);
            float p_spread = read_f32(&ptr);
            float start_size = read_f32(&ptr);
            float end_size = read_f32(&ptr);
            float speed = read_f32(&ptr);
            float rate = read_f32(&ptr);
            Color outer_color;
            outer_color.r = read_u8(&ptr);
            outer_color.g = read_u8(&ptr);
            outer_color.b = read_u8(&ptr);
            outer_color.a = 255;
            Color inner_color;
            inner_color.r = read_u8(&ptr);
            inner_color.g = read_u8(&ptr);
            inner_color.b = read_u8(&ptr);
            inner_color.a = 255;
            uint8_t loop = read_u8(&ptr);

            ParticleComponent* particle = ParticleComponent_get(entity);
            if (!particle) {
                particle = ParticleComponent_add(entity, p_angle, p_spread,
                    start_size, end_size, speed, rate, outer_color, inner_color);
            }
            particle->angle = p_angle;
            particle->spread = p_spread;
            particle->start_size = start_size;
            particle->end_size = end_size;
            particle->speed = speed;
            particle->rate = rate;
            particle->outer_color = outer_color;
            particle->inner_color = inner_color;
            particle->loop = loop;
            if (loop) particle->enabled = true;
        } else {
            CHECK(4 + 4 + 4 + 1 + 4);
            float start_size = read_f32(&ptr);
            float width = read_f32(&ptr);
            float height = read_f32(&ptr);
            uint8_t loop = read_u8(&ptr);
            float rate = read_f32(&ptr);

            ParticleComponent* particle = ParticleComponent_get(entity);
            if (!particle) {
                particle = ParticleComponent_add_type(entity, (ParticleType)type, start_size);
            }
            particle->start_size = start_size;
            particle->width = width;
            particle->height = height;
            particle->loop = loop;
            if (loop) particle->enabled = true;
            particle->rate = rate;
        }
    }

    // --- Light ---
    if (flags & BFLAG_HAS_LIGHT) {
        CHECK(4 + 4 + 3 + 4 + 4 + 1 + 4 + 4);
        float range = read_f32(&ptr);
        float l_angle = read_f32(&ptr);
        Color color;
        color.r = read_u8(&ptr);
        color.g = read_u8(&ptr);
        color.b = read_u8(&ptr);
        color.a = 255;
        float max_brightness = read_f32(&ptr);
        float speed = read_f32(&ptr);
        uint8_t enabled = read_u8(&ptr);
        float flicker = read_f32(&ptr);
        // Runtime state
        float brightness = read_f32(&ptr);

        LightComponent* light = LightComponent_get(entity);
        if (!light) {
            light = LightComponent_add(entity, range, l_angle, color, max_brightness, speed);
        }
        light->range = range;
        light->angle = l_angle;
        light->color = color;
        light->max_brightness = max_brightness;
        light->speed = speed;
        light->enabled = enabled;
        light->flicker = flicker;
        light->brightness = brightness;
    }

    // --- Sound ---
    if (flags & BFLAG_HAS_SOUND) {
        Filename hit_sound, loop_sound;
        int sn;
        sn = read_string(ptr, (int)(end - ptr), hit_sound, sizeof(hit_sound));
        if (!sn) return 0; ptr += sn;
        sn = read_string(ptr, (int)(end - ptr), loop_sound, sizeof(loop_sound));
        if (!sn) return 0; ptr += sn;

        SoundComponent* sound = SoundComponent_get(entity);
        if (!sound) {
            sound = SoundComponent_add(entity, hit_sound);
        }
        strcpy(sound->hit_sound, hit_sound);
        strcpy(sound->loop_sound, loop_sound);
    }

    // --- Ammo ---
    if (flags & BFLAG_HAS_AMMO) {
        CHECK(1 + 2);
        uint8_t type = read_u8(&ptr);
        int16_t size = read_i16(&ptr);

        AmmoComponent* ammo = AmmoComponent_get(entity);
        if (!ammo) {
            ammo = AmmoComponent_add(entity, (AmmoType)type);
        }
        ammo->type = (AmmoType)type;
        ammo->size = (int)size;
    }

    // --- Animation ---
    if (flags & BFLAG_HAS_ANIMATION) {
        CHECK(2 + 4 + 1 + 4 + 2);
        int16_t frames = read_i16(&ptr);
        float framerate = read_f32(&ptr);
        uint8_t play_once = read_u8(&ptr);
        float wind_factor = read_f32(&ptr);
        // Runtime state
        int16_t current_frame = read_i16(&ptr);

        AnimationComponent* anim = AnimationComponent_get(entity);
        if (!anim) {
            anim = AnimationComponent_add(entity, (int)frames);
        }
        anim->frames = (int)frames;
        anim->framerate = framerate;
        anim->play_once = play_once;
        anim->wind_factor = wind_factor;
        anim->current_frame = (int)current_frame;
    }

    // --- Door ---
    if (flags & BFLAG_HAS_DOOR) {
        CHECK(2 + 1 + 1);
        int16_t price = read_i16(&ptr);
        uint8_t locked = read_u8(&ptr);
        uint8_t key = read_u8(&ptr);

        DoorComponent* door = DoorComponent_get(entity);
        if (!door) {
            door = DoorComponent_add(entity, (int)price);
        }
        door->price = (int)price;
        door->locked = locked;
        door->key = (int)key;
    }

    // --- Joint ---
    if (flags & BFLAG_HAS_JOINT) {
        CHECK(2 + 4*4);
        int16_t j_parent = read_i16(&ptr);
        float min_length = read_f32(&ptr);
        float max_length = read_f32(&ptr);
        float strength = read_f32(&ptr);
        float max_angle = read_f32(&ptr);

        JointComponent* joint = JointComponent_get(entity);
        if (!joint) {
            joint = JointComponent_add(entity, (int)j_parent, min_length, max_length, strength);
        }
        joint->parent = (int)j_parent;    // raw host ID; remapped by post-pass
        joint->min_length = min_length;
        joint->max_length = max_length;
        joint->strength = strength;
        joint->max_angle = max_angle;
    }

    // --- Vehicle ---
    if (flags & BFLAG_HAS_VEHICLE) {
        CHECK(4 + 1 + 4 + 4 + 4);
        float max_fuel = read_f32(&ptr);
        uint8_t size = read_u8(&ptr);
        float v_acceleration = read_f32(&ptr);
        float v_max_speed = read_f32(&ptr);
        float turning = read_f32(&ptr);
        int seat_count = (size < 4) ? size : 4;
        CHECK(seat_count * 8 + 4 + 1 + 4*2);
        Vector2f seats[4] = {0};
        for (int i = 0; i < seat_count; i++) {
            seats[i].x = read_f32(&ptr);
            seats[i].y = read_f32(&ptr);
        }
        // Runtime state
        float fuel = read_f32(&ptr);
        uint8_t on_road = read_u8(&ptr);
        int16_t riders[4];
        for (int i = 0; i < 4; i++) riders[i] = read_i16(&ptr);

        VehicleComponent* vehicle = VehicleComponent_get(entity);
        if (!vehicle) {
            vehicle = VehicleComponent_add(entity, max_fuel);
        }
        vehicle->max_fuel = max_fuel;
        vehicle->size = (int)size;
        vehicle->acceleration = v_acceleration;
        vehicle->max_speed = v_max_speed;
        vehicle->turning = turning;
        for (int i = 0; i < seat_count; i++) {
            vehicle->seats[i] = seats[i];
        }
        vehicle->fuel = fuel;
        vehicle->on_road = on_road != 0;
        for (int i = 0; i < 4; i++) {
            vehicle->riders[i] = (int)riders[i];  // raw host IDs; remapped by post-pass
        }
    }

    // --- Text ---
    if (flags & BFLAG_HAS_TEXT) {
        String source_string;
        int sn = read_string(ptr, (int)(end - ptr), source_string, sizeof(source_string));
        if (!sn) return 0; ptr += sn;
        CHECK(2 + 3);
        int16_t size = read_i16(&ptr);
        Color color;
        color.r = read_u8(&ptr);
        color.g = read_u8(&ptr);
        color.b = read_u8(&ptr);
        color.a = 255;

        TextComponent* text = TextComponent_get(entity);
        if (!text) {
            text = TextComponent_add(entity, source_string, (int)size, color);
        } else {
            strcpy(text->source_string, source_string);
            text->size = (int)size;
            text->color = color;
        }
    }

#undef CHECK

    return (int)(ptr - buf);
}


// ============================================================
// Skip (consume bytes without creating anything)
// ============================================================

int binary_skip_entity(const uint8_t* buf, int buf_size) {
    const uint8_t* ptr = buf;
    const uint8_t* end = buf + buf_size;

#define SKIP_CHECK(need) do { if (ptr + (need) > end) return 0; } while(0)

    // --- Coordinate (always present, 30 bytes) ---
    SKIP_CHECK(30);
    ptr += 30;

    // --- Flags ---
    SKIP_CHECK(4);
    uint32_t flags;
    memcpy(&flags, ptr, 4);
    ptr += 4;

    // --- Image ---
    if (flags & BFLAG_HAS_IMAGE) {
        if (ptr >= end) return 0;
        uint8_t slen = *ptr;
        ptr += 1 + slen;
        SKIP_CHECK(4 + 4 + 1 + 4 + 4);
        ptr += 4 + 4 + 1 + 4 + 4;
    }

    // --- Physics (creation + runtime: +8 bytes for velocity) ---
    if (flags & BFLAG_HAS_PHYSICS) {
        SKIP_CHECK(4 + 4 + 4 + 4 + 4 + 4);
        ptr += 4 + 4 + 4 + 4 + 4 + 4;
    }

    // --- Collider ---
    if (flags & BFLAG_HAS_COLLIDER) {
        SKIP_CHECK(1 + 4 + 4 + 1 + 1 + 1);
        ptr += 1 + 4 + 4 + 1 + 1 + 1;
    }

    // --- Player (runtime state) ---
    if (flags & BFLAG_HAS_PLAYER) {
        // state(1) + money(4) + item(1) + inventory(8) + ammo(8) + keys(6) + arms(2) + vehicle(2) + target(2) + use_timer(4) + grabbed_item(2) + won(1) + money_timer(4) + money_increment(4)
        SKIP_CHECK(1 + 4 + 1 + 8 + 8 + 6 + 2 + 2 + 2 + 4 + 2 + 1 + 4 + 4);
        ptr += 1 + 4 + 1 + 8 + 8 + 6 + 2 + 2 + 2 + 4 + 2 + 1 + 4 + 4;
    }

    // --- Enemy (creation + runtime: +5 bytes) ---
    if (flags & BFLAG_HAS_ENEMY) {
        SKIP_CHECK(4*7 + 1 + 1 + 4 + 4 + 4 + 1 + 2 + 2);
        ptr += 4*7 + 1 + 1 + 4 + 4 + 4 + 1 + 2 + 2;
    }

    // --- Health (creation + runtime: +2 bytes) ---
    if (flags & BFLAG_HAS_HEALTH) {
        SKIP_CHECK(2 + 2);
        ptr += 2 + 2;
        for (int i = 0; i < 3; i++) {
            if (ptr >= end) return 0;
            uint8_t slen = *ptr;
            if (ptr + 1 + slen > end) return 0;
            ptr += 1 + slen;
        }
        SKIP_CHECK(4 * DAMAGE_COUNT + 1 + 1);
        ptr += 4 * DAMAGE_COUNT + 1 + 1;
    }

    // --- Weapon (creation + runtime: +11 bytes) ---
    if (flags & BFLAG_HAS_WEAPON) {
        SKIP_CHECK(4 + 2 + 2 + 4 + 2 + 4 + 4 + 4 + 4 + 4 + 1);
        ptr += 4 + 2 + 2 + 4 + 2 + 4 + 4 + 4 + 4 + 4 + 1;
        // sound string
        if (ptr >= end) return 0;
        uint8_t slen = *ptr;
        if (ptr + 1 + slen > end) return 0;
        ptr += 1 + slen;
        // automatic + penetration + runtime (recoil, cooldown, magazine, reloading)
        SKIP_CHECK(1 + 2 + 4 + 4 + 2 + 1);
        ptr += 1 + 2 + 4 + 4 + 2 + 1;
    }

    // --- Item (creation + runtime: +10 bytes for attachments) ---
    if (flags & BFLAG_HAS_ITEM) {
        SKIP_CHECK(1 + 2);
        ptr += 1 + 2;
        // name string
        if (ptr >= end) return 0;
        uint8_t slen = *ptr;
        if (ptr + 1 + slen > end) return 0;
        ptr += 1 + slen;
        SKIP_CHECK(1 + 2 + 4 + 5*2);
        ptr += 1 + 2 + 4 + 5*2;
    }

    // --- Waypoint --- (no data)

    // --- Particle ---
    if (flags & BFLAG_HAS_PARTICLE) {
        SKIP_CHECK(1);
        uint8_t type = *ptr;
        ptr += 1;
        if (type == PARTICLE_NONE) {
            SKIP_CHECK(4*6 + 6 + 1);
            ptr += 4*6 + 6 + 1;
        } else {
            SKIP_CHECK(4 + 4 + 4 + 1 + 4);
            ptr += 4 + 4 + 4 + 1 + 4;
        }
    }

    // --- Light (creation + runtime: +4 bytes for brightness) ---
    if (flags & BFLAG_HAS_LIGHT) {
        SKIP_CHECK(4 + 4 + 3 + 4 + 4 + 1 + 4 + 4);
        ptr += 4 + 4 + 3 + 4 + 4 + 1 + 4 + 4;
    }

    // --- Sound ---
    if (flags & BFLAG_HAS_SOUND) {
        for (int i = 0; i < 2; i++) {
            if (ptr >= end) return 0;
            uint8_t slen = *ptr;
            if (ptr + 1 + slen > end) return 0;
            ptr += 1 + slen;
        }
    }

    // --- Ammo ---
    if (flags & BFLAG_HAS_AMMO) {
        SKIP_CHECK(1 + 2);
        ptr += 1 + 2;
    }

    // --- Animation (creation + runtime: +2 bytes for current_frame) ---
    if (flags & BFLAG_HAS_ANIMATION) {
        SKIP_CHECK(2 + 4 + 1 + 4 + 2);
        ptr += 2 + 4 + 1 + 4 + 2;
    }

    // --- Door ---
    if (flags & BFLAG_HAS_DOOR) {
        SKIP_CHECK(2 + 1 + 1);
        ptr += 2 + 1 + 1;
    }

    // --- Joint ---
    if (flags & BFLAG_HAS_JOINT) {
        SKIP_CHECK(2 + 4*4);
        ptr += 2 + 4*4;
    }

    // --- Vehicle (creation + runtime) ---
    if (flags & BFLAG_HAS_VEHICLE) {
        SKIP_CHECK(4 + 1 + 4 + 4 + 4);
        uint8_t size = ptr[4]; // size is at offset 4 (after max_fuel float)
        ptr += 4 + 1 + 4 + 4 + 4;
        int seat_count = (size < 4) ? size : 4;
        // seats + runtime (fuel, on_road, riders[4])
        SKIP_CHECK(seat_count * 8 + 4 + 1 + 4*2);
        ptr += seat_count * 8 + 4 + 1 + 4*2;
    }

    // --- Text ---
    if (flags & BFLAG_HAS_TEXT) {
        if (ptr >= end) return 0;
        uint8_t slen = *ptr;
        if (ptr + 1 + slen > end) return 0;
        ptr += 1 + slen;
        SKIP_CHECK(2 + 3);
        ptr += 2 + 3;
    }

#undef SKIP_CHECK

    return (int)(ptr - buf);
}
