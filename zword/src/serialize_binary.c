#include <string.h>

#include "serialize_binary.h"
#include "component.h"
#include "particle.h"
#include "image.h"
#include "util.h"


typedef struct {
    uint8_t* ptr;
    int remaining;
} BinaryWriteCursor;

typedef struct {
    const uint8_t* ptr;
    const uint8_t* end;
} BinaryReadCursor;


static bool write_bytes(BinaryWriteCursor* cursor, const void* data, int size) {
    if (cursor->remaining < size) {
        return false;
    }

    memcpy(cursor->ptr, data, size);
    cursor->ptr += size;
    cursor->remaining -= size;
    return true;
}


static bool read_bytes(BinaryReadCursor* cursor, void* out, int size) {
    if (cursor->ptr + size > cursor->end) {
        return false;
    }

    memcpy(out, cursor->ptr, size);
    cursor->ptr += size;
    return true;
}


static bool skip_bytes(BinaryReadCursor* cursor, int size) {
    if (cursor->ptr + size > cursor->end) {
        return false;
    }

    cursor->ptr += size;
    return true;
}


static bool write_string_value(BinaryWriteCursor* cursor, const char* str) {
    uint8_t len = (uint8_t)strlen(str);
    if (!write_bytes(cursor, &len, 1)) {
        return false;
    }
    return write_bytes(cursor, str, (int)len);
}


static bool read_string_value(BinaryReadCursor* cursor, char* out, int out_size) {
    if (cursor->ptr >= cursor->end) {
        return false;
    }

    uint8_t len = cursor->ptr[0];
    if (!skip_bytes(cursor, 1)) {
        return false;
    }
    if (cursor->ptr + len > cursor->end) {
        return false;
    }

    int copy = (len < out_size - 1) ? len : out_size - 1;
    memcpy(out, cursor->ptr, copy);
    out[copy] = '\0';
    cursor->ptr += len;
    return true;
}


static bool skip_string_value(BinaryReadCursor* cursor) {
    if (cursor->ptr >= cursor->end) {
        return false;
    }

    uint8_t len = cursor->ptr[0];
    return skip_bytes(cursor, 1 + (int)len);
}


static bool write_f32_value(BinaryWriteCursor* cursor, float value) {
    return write_bytes(cursor, &value, 4);
}


static bool write_i32_value(BinaryWriteCursor* cursor, int32_t value) {
    return write_bytes(cursor, &value, 4);
}


static bool write_u32_value(BinaryWriteCursor* cursor, uint32_t value) {
    return write_bytes(cursor, &value, 4);
}


static bool write_i16_value(BinaryWriteCursor* cursor, int16_t value) {
    return write_bytes(cursor, &value, 2);
}


static bool write_u8_value(BinaryWriteCursor* cursor, uint8_t value) {
    return write_bytes(cursor, &value, 1);
}


static bool read_f32_value(BinaryReadCursor* cursor, float* value) {
    return read_bytes(cursor, value, 4);
}


static bool read_i32_value(BinaryReadCursor* cursor, int32_t* value) {
    return read_bytes(cursor, value, 4);
}


static bool read_u32_value(BinaryReadCursor* cursor, uint32_t* value) {
    return read_bytes(cursor, value, 4);
}


static bool read_i16_value(BinaryReadCursor* cursor, int16_t* value) {
    return read_bytes(cursor, value, 2);
}


static bool read_u8_value(BinaryReadCursor* cursor, uint8_t* value) {
    return read_bytes(cursor, value, 1);
}


static bool CoordinateComponent_serialize_binary(BinaryWriteCursor* cursor, CoordinateComponent* coord) {
    if (!write_f32_value(cursor, coord->position.x)) return false;
    if (!write_f32_value(cursor, coord->position.y)) return false;
    if (!write_f32_value(cursor, coord->angle)) return false;
    if (!write_i16_value(cursor, (int16_t)coord->parent)) return false;
    if (!write_f32_value(cursor, coord->scale.x)) return false;
    if (!write_f32_value(cursor, coord->scale.y)) return false;
    if (!write_f32_value(cursor, coord->lifetime)) return false;
    return true;
}


static bool CoordinateComponent_deserialize_binary(BinaryReadCursor* cursor, int entity, bool* smooth) {
    float pos_x;
    float pos_y;
    float angle;
    int16_t parent;
    float scale_x;
    float scale_y;
    float lifetime;

    if (!read_f32_value(cursor, &pos_x)) return false;
    if (!read_f32_value(cursor, &pos_y)) return false;
    if (!read_f32_value(cursor, &angle)) return false;
    if (!read_i16_value(cursor, &parent)) return false;
    if (!read_f32_value(cursor, &scale_x)) return false;
    if (!read_f32_value(cursor, &scale_y)) return false;
    if (!read_f32_value(cursor, &lifetime)) return false;

    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (!coord) {
        return true;
    }

    Vector2f new_pos = vec(pos_x, pos_y);
    float distance = dist(new_pos, coord->position);
    if (coord->parent != (int)parent || distance > 1.0f) {
        *smooth = false;
    }

    if (*smooth) {
        float t = 0.5f;
        coord->position.x = lerp(coord->position.x, pos_x, t);
        coord->position.y = lerp(coord->position.y, pos_y, t);
        coord->angle = lerp_angle(coord->angle, angle, t);
    } else {
        coord->position.x = pos_x;
        coord->position.y = pos_y;
        coord->angle = angle;
        coord->previous.position.x = pos_x;
        coord->previous.position.y = pos_y;
        coord->previous.angle = angle;
    }

    coord->parent = (int)parent;
    coord->scale.x = scale_x;
    coord->scale.y = scale_y;
    coord->lifetime = lifetime;
    return true;
}


static bool ImageComponent_serialize_binary(BinaryWriteCursor* cursor, ImageComponent* image) {
    if (!write_string_value(cursor, image->filename)) return false;
    if (!write_f32_value(cursor, image->width)) return false;
    if (!write_f32_value(cursor, image->height)) return false;
    if (!write_u8_value(cursor, (uint8_t)image->layer)) return false;
    if (!write_f32_value(cursor, image->alpha)) return false;
    if (!write_f32_value(cursor, image->shine)) return false;
    return true;
}


static bool ImageComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    Filename filename;
    float width;
    float height;
    uint8_t layer;
    float alpha;
    float shine;

    if (!read_string_value(cursor, filename, sizeof(filename))) return false;
    if (!read_f32_value(cursor, &width)) return false;
    if (!read_f32_value(cursor, &height)) return false;
    if (!read_u8_value(cursor, &layer)) return false;
    if (!read_f32_value(cursor, &alpha)) return false;
    if (!read_f32_value(cursor, &shine)) return false;

    ImageComponent* image = ImageComponent_get(entity);
    if (image) {
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

    return true;
}


static bool PhysicsComponent_serialize_binary(BinaryWriteCursor* cursor, PhysicsComponent* physics) {
    if (!write_f32_value(cursor, physics->mass)) return false;
    if (!write_f32_value(cursor, physics->bounce)) return false;
    if (!write_f32_value(cursor, physics->max_speed)) return false;
    if (!write_f32_value(cursor, physics->angular_drag)) return false;
    if (!write_f32_value(cursor, physics->velocity.x)) return false;
    if (!write_f32_value(cursor, physics->velocity.y)) return false;
    return true;
}


static bool PhysicsComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    float mass;
    float bounce;
    float max_speed;
    float angular_drag;
    float vel_x;
    float vel_y;

    if (!read_f32_value(cursor, &mass)) return false;
    if (!read_f32_value(cursor, &bounce)) return false;
    if (!read_f32_value(cursor, &max_speed)) return false;
    if (!read_f32_value(cursor, &angular_drag)) return false;
    if (!read_f32_value(cursor, &vel_x)) return false;
    if (!read_f32_value(cursor, &vel_y)) return false;

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
    return true;
}


static bool ColliderComponent_serialize_binary(BinaryWriteCursor* cursor, ColliderComponent* collider) {
    if (!write_u8_value(cursor, (uint8_t)collider->type)) return false;
    if (!write_f32_value(cursor, collider->width)) return false;
    if (!write_f32_value(cursor, collider->height)) return false;
    if (!write_u8_value(cursor, (uint8_t)collider->group)) return false;
    if (!write_u8_value(cursor, (uint8_t)collider->enabled)) return false;
    if (!write_u8_value(cursor, (uint8_t)collider->trigger_type)) return false;
    return true;
}


static bool ColliderComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    uint8_t type;
    float width;
    float height;
    uint8_t group;
    uint8_t enabled;
    uint8_t trigger_type;

    if (!read_u8_value(cursor, &type)) return false;
    if (!read_f32_value(cursor, &width)) return false;
    if (!read_f32_value(cursor, &height)) return false;
    if (!read_u8_value(cursor, &group)) return false;
    if (!read_u8_value(cursor, &enabled)) return false;
    if (!read_u8_value(cursor, &trigger_type)) return false;

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
    return true;
}


static bool PlayerComponent_serialize_binary(BinaryWriteCursor* cursor, PlayerComponent* player) {
    if (!write_u8_value(cursor, (uint8_t)player->state)) return false;
    if (!write_i32_value(cursor, (int32_t)player->money)) return false;
    if (!write_u8_value(cursor, (uint8_t)player->item)) return false;
    for (int i = 0; i < 4; i++) {
        if (!write_i16_value(cursor, (int16_t)player->inventory[i])) return false;
    }
    for (int i = 0; i < 4; i++) {
        if (!write_i16_value(cursor, (int16_t)player->ammo[i])) return false;
    }
    for (int i = 0; i < 3; i++) {
        if (!write_i16_value(cursor, (int16_t)player->keys[i])) return false;
    }
    if (!write_i16_value(cursor, (int16_t)player->arms)) return false;
    if (!write_i16_value(cursor, (int16_t)player->vehicle)) return false;
    if (!write_i16_value(cursor, (int16_t)player->target)) return false;
    if (!write_f32_value(cursor, player->use_timer)) return false;
    if (!write_i16_value(cursor, (int16_t)player->grabbed_item)) return false;
    if (!write_u8_value(cursor, player->won ? 1 : 0)) return false;
    if (!write_f32_value(cursor, player->money_timer)) return false;
    if (!write_i32_value(cursor, (int32_t)player->money_increment)) return false;
    if (!write_i32_value(cursor, (int32_t)player->kills)) return false;
    if (!write_i32_value(cursor, (int32_t)player->deaths)) return false;
    return true;
}


static bool PlayerComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    uint8_t state;
    int32_t money;
    uint8_t item_slot;
    int16_t inventory[4];
    int16_t ammo_slots[4];
    int16_t keys[3];
    int16_t arms;
    int16_t vehicle_id;
    int16_t target;
    float use_timer;
    int16_t grabbed_item;
    uint8_t won;
    float money_timer;
    int32_t money_increment;
    int32_t kills;
    int32_t deaths;

    if (!read_u8_value(cursor, &state)) return false;
    if (!read_i32_value(cursor, &money)) return false;
    if (!read_u8_value(cursor, &item_slot)) return false;
    for (int i = 0; i < 4; i++) {
        if (!read_i16_value(cursor, &inventory[i])) return false;
    }
    for (int i = 0; i < 4; i++) {
        if (!read_i16_value(cursor, &ammo_slots[i])) return false;
    }
    for (int i = 0; i < 3; i++) {
        if (!read_i16_value(cursor, &keys[i])) return false;
    }
    if (!read_i16_value(cursor, &arms)) return false;
    if (!read_i16_value(cursor, &vehicle_id)) return false;
    if (!read_i16_value(cursor, &target)) return false;
    if (!read_f32_value(cursor, &use_timer)) return false;
    if (!read_i16_value(cursor, &grabbed_item)) return false;
    if (!read_u8_value(cursor, &won)) return false;
    if (!read_f32_value(cursor, &money_timer)) return false;
    if (!read_i32_value(cursor, &money_increment)) return false;
    if (!read_i32_value(cursor, &kills)) return false;
    if (!read_i32_value(cursor, &deaths)) return false;

    PlayerComponent* player = PlayerComponent_get(entity);
    if (!player) {
        player = PlayerComponent_add(entity);
    }

    player->state = (PlayerState)state;
    player->money = (int)money;
    player->item = (int)item_slot;
    for (int i = 0; i < 4; i++) {
        player->inventory[i] = (int)inventory[i];
    }
    for (int i = 0; i < 4; i++) {
        player->ammo[i] = (int)ammo_slots[i];
    }
    for (int i = 0; i < 3; i++) {
        player->keys[i] = (int)keys[i];
    }
    player->arms = (int)arms;
    player->vehicle = (int)vehicle_id;
    player->target = (int)target;
    player->use_timer = use_timer;
    player->grabbed_item = (int)grabbed_item;
    player->won = won != 0;
    player->money_timer = money_timer;
    player->money_increment = (int)money_increment;
    player->kills = (int)kills;
    player->deaths = (int)deaths;
    return true;
}


static bool EnemyComponent_serialize_binary(BinaryWriteCursor* cursor, EnemyComponent* enemy) {
    if (!write_f32_value(cursor, enemy->fov)) return false;
    if (!write_f32_value(cursor, enemy->idle_speed)) return false;
    if (!write_f32_value(cursor, enemy->walk_speed)) return false;
    if (!write_f32_value(cursor, enemy->run_speed)) return false;
    if (!write_f32_value(cursor, enemy->attack_delay)) return false;
    if (!write_f32_value(cursor, enemy->turn_speed)) return false;
    if (!write_f32_value(cursor, enemy->attack_angle)) return false;
    if (!write_u8_value(cursor, (uint8_t)enemy->spawner)) return false;
    if (!write_u8_value(cursor, (uint8_t)enemy->boss)) return false;
    if (!write_i32_value(cursor, (int32_t)enemy->bounty)) return false;
    if (!write_f32_value(cursor, enemy->vision_range)) return false;
    if (!write_f32_value(cursor, enemy->acceleration)) return false;
    if (!write_u8_value(cursor, (uint8_t)enemy->state)) return false;
    if (!write_i16_value(cursor, (int16_t)enemy->weapon)) return false;
    if (!write_i16_value(cursor, (int16_t)enemy->target)) return false;
    return true;
}


static bool EnemyComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    float fov;
    float idle_speed;
    float walk_speed;
    float run_speed;
    float attack_delay;
    float turn_speed;
    float attack_angle;
    uint8_t spawner;
    uint8_t boss;
    int32_t bounty;
    float vision_range;
    float acceleration;
    uint8_t state;
    int16_t weapon_id;
    int16_t target;

    if (!read_f32_value(cursor, &fov)) return false;
    if (!read_f32_value(cursor, &idle_speed)) return false;
    if (!read_f32_value(cursor, &walk_speed)) return false;
    if (!read_f32_value(cursor, &run_speed)) return false;
    if (!read_f32_value(cursor, &attack_delay)) return false;
    if (!read_f32_value(cursor, &turn_speed)) return false;
    if (!read_f32_value(cursor, &attack_angle)) return false;
    if (!read_u8_value(cursor, &spawner)) return false;
    if (!read_u8_value(cursor, &boss)) return false;
    if (!read_i32_value(cursor, &bounty)) return false;
    if (!read_f32_value(cursor, &vision_range)) return false;
    if (!read_f32_value(cursor, &acceleration)) return false;
    if (!read_u8_value(cursor, &state)) return false;
    if (!read_i16_value(cursor, &weapon_id)) return false;
    if (!read_i16_value(cursor, &target)) return false;

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
    enemy->weapon = (int)weapon_id;
    enemy->target = (int)target;
    return true;
}


static bool HealthComponent_serialize_binary(BinaryWriteCursor* cursor, HealthComponent* health) {
    if (!write_i16_value(cursor, (int16_t)health->health)) return false;
    if (!write_i16_value(cursor, (int16_t)health->max_health)) return false;
    if (!write_string_value(cursor, health->dead_image)) return false;
    if (!write_string_value(cursor, health->decal)) return false;
    if (!write_string_value(cursor, health->die_sound)) return false;
    for (int i = 0; i < DAMAGE_COUNT; i++) {
        if (!write_f32_value(cursor, health->damage_factor[i])) return false;
    }
    if (!write_u8_value(cursor, health->dead ? 1 : 0)) return false;
    if (!write_u8_value(cursor, (uint8_t)health->status.type)) return false;
    return true;
}


static bool HealthComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    int16_t hp;
    int16_t max_hp;
    Filename dead_image;
    Filename decal;
    Filename die_sound;
    float damage_factor[DAMAGE_COUNT];
    uint8_t dead;
    uint8_t status_type;

    if (!read_i16_value(cursor, &hp)) return false;
    if (!read_i16_value(cursor, &max_hp)) return false;
    if (!read_string_value(cursor, dead_image, sizeof(dead_image))) return false;
    if (!read_string_value(cursor, decal, sizeof(decal))) return false;
    if (!read_string_value(cursor, die_sound, sizeof(die_sound))) return false;
    for (int i = 0; i < DAMAGE_COUNT; i++) {
        if (!read_f32_value(cursor, &damage_factor[i])) return false;
    }
    if (!read_u8_value(cursor, &dead)) return false;
    if (!read_u8_value(cursor, &status_type)) return false;

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
    return true;
}


static bool WeaponComponent_serialize_binary(BinaryWriteCursor* cursor, WeaponComponent* weapon) {
    if (!write_f32_value(cursor, weapon->fire_rate)) return false;
    if (!write_i16_value(cursor, (int16_t)weapon->damage)) return false;
    if (!write_i16_value(cursor, (int16_t)weapon->shots)) return false;
    if (!write_f32_value(cursor, weapon->spread)) return false;
    if (!write_i16_value(cursor, (int16_t)weapon->max_magazine)) return false;
    if (!write_f32_value(cursor, weapon->recoil_up)) return false;
    if (!write_f32_value(cursor, weapon->max_recoil)) return false;
    if (!write_f32_value(cursor, weapon->range)) return false;
    if (!write_f32_value(cursor, weapon->sound_range)) return false;
    if (!write_f32_value(cursor, weapon->reload_time)) return false;
    if (!write_u8_value(cursor, (uint8_t)weapon->ammo_type)) return false;
    if (!write_string_value(cursor, weapon->sound)) return false;
    if (!write_u8_value(cursor, (uint8_t)weapon->automatic)) return false;
    if (!write_i16_value(cursor, (int16_t)weapon->penetration)) return false;
    if (!write_f32_value(cursor, weapon->recoil)) return false;
    if (!write_f32_value(cursor, weapon->cooldown)) return false;
    if (!write_i16_value(cursor, (int16_t)weapon->magazine)) return false;
    if (!write_u8_value(cursor, weapon->reloading ? 1 : 0)) return false;
    return true;
}


static bool WeaponComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    float fire_rate;
    int16_t damage;
    int16_t shots;
    float spread;
    int16_t max_magazine;
    float recoil_up;
    float max_recoil;
    float range;
    float sound_range;
    float reload_time;
    uint8_t ammo_type;
    Filename sound_name;
    uint8_t automatic;
    int16_t penetration;
    float recoil;
    float cooldown;
    int16_t magazine;
    uint8_t reloading;

    if (!read_f32_value(cursor, &fire_rate)) return false;
    if (!read_i16_value(cursor, &damage)) return false;
    if (!read_i16_value(cursor, &shots)) return false;
    if (!read_f32_value(cursor, &spread)) return false;
    if (!read_i16_value(cursor, &max_magazine)) return false;
    if (!read_f32_value(cursor, &recoil_up)) return false;
    if (!read_f32_value(cursor, &max_recoil)) return false;
    if (!read_f32_value(cursor, &range)) return false;
    if (!read_f32_value(cursor, &sound_range)) return false;
    if (!read_f32_value(cursor, &reload_time)) return false;
    if (!read_u8_value(cursor, &ammo_type)) return false;
    if (!read_string_value(cursor, sound_name, sizeof(sound_name))) return false;
    if (!read_u8_value(cursor, &automatic)) return false;
    if (!read_i16_value(cursor, &penetration)) return false;
    if (!read_f32_value(cursor, &recoil)) return false;
    if (!read_f32_value(cursor, &cooldown)) return false;
    if (!read_i16_value(cursor, &magazine)) return false;
    if (!read_u8_value(cursor, &reloading)) return false;

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
    return true;
}


static bool ItemComponent_serialize_binary(BinaryWriteCursor* cursor, ItemComponent* item) {
    if (!write_u8_value(cursor, (uint8_t)item->size)) return false;
    if (!write_i16_value(cursor, (int16_t)item->price)) return false;
    if (!write_string_value(cursor, item->name)) return false;
    if (!write_u8_value(cursor, (uint8_t)item->type)) return false;
    if (!write_i16_value(cursor, (int16_t)item->value)) return false;
    if (!write_f32_value(cursor, item->use_time)) return false;
    for (int i = 0; i < 5; i++) {
        if (!write_i16_value(cursor, (int16_t)item->attachments[i])) return false;
    }
    return true;
}


static bool ItemComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    uint8_t size;
    int16_t price;
    ButtonText name;
    uint8_t type;
    int16_t value;
    float use_time;
    int16_t attachments[5];

    if (!read_u8_value(cursor, &size)) return false;
    if (!read_i16_value(cursor, &price)) return false;
    if (!read_string_value(cursor, name, sizeof(name))) return false;
    if (!read_u8_value(cursor, &type)) return false;
    if (!read_i16_value(cursor, &value)) return false;
    if (!read_f32_value(cursor, &use_time)) return false;
    for (int i = 0; i < 5; i++) {
        if (!read_i16_value(cursor, &attachments[i])) return false;
    }

    ItemComponent* item = ItemComponent_get(entity);
    if (!item) {
        item = ItemComponent_add(entity, (int)size, (int)price, name);
    }

    item->size = (int)size;
    item->price = (int)price;
    strcpy(item->name, name);
    item->type = (ItemType)type;
    item->value = (int)value;
    item->use_time = use_time;
    for (int i = 0; i < 5; i++) {
        item->attachments[i] = (int)attachments[i];
    }
    return true;
}


static bool WaypointComponent_serialize_binary(BinaryWriteCursor* cursor, WaypointComponent* waypoint) {
    (void)cursor;
    (void)waypoint;
    return true;
}


static bool WaypointComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    (void)cursor;

    WaypointComponent* waypoint = WaypointComponent_get(entity);
    if (!waypoint) {
        WaypointComponent_add(entity);
    }

    return true;
}


static bool ParticleComponent_serialize_binary(BinaryWriteCursor* cursor, ParticleComponent* particle) {
    if (!write_u8_value(cursor, (uint8_t)particle->type)) return false;

    if (particle->type == PARTICLE_NONE) {
        if (!write_f32_value(cursor, particle->angle)) return false;
        if (!write_f32_value(cursor, particle->spread)) return false;
        if (!write_f32_value(cursor, particle->start_size)) return false;
        if (!write_f32_value(cursor, particle->end_size)) return false;
        if (!write_f32_value(cursor, particle->speed)) return false;
        if (!write_f32_value(cursor, particle->rate)) return false;
        if (!write_u8_value(cursor, (uint8_t)particle->outer_color.r)) return false;
        if (!write_u8_value(cursor, (uint8_t)particle->outer_color.g)) return false;
        if (!write_u8_value(cursor, (uint8_t)particle->outer_color.b)) return false;
        if (!write_u8_value(cursor, (uint8_t)particle->inner_color.r)) return false;
        if (!write_u8_value(cursor, (uint8_t)particle->inner_color.g)) return false;
        if (!write_u8_value(cursor, (uint8_t)particle->inner_color.b)) return false;
        if (!write_u8_value(cursor, (uint8_t)particle->loop)) return false;
    } else {
        if (!write_f32_value(cursor, particle->start_size)) return false;
        if (!write_f32_value(cursor, particle->width)) return false;
        if (!write_f32_value(cursor, particle->height)) return false;
        if (!write_u8_value(cursor, (uint8_t)particle->loop)) return false;
        if (!write_f32_value(cursor, particle->rate)) return false;
    }

    return true;
}


static bool ParticleComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    uint8_t type;

    if (!read_u8_value(cursor, &type)) return false;

    if (type == PARTICLE_NONE) {
        float angle;
        float spread;
        float start_size;
        float end_size;
        float speed;
        float rate;
        Color outer_color;
        Color inner_color;
        uint8_t outer_r;
        uint8_t outer_g;
        uint8_t outer_b;
        uint8_t inner_r;
        uint8_t inner_g;
        uint8_t inner_b;
        uint8_t loop;

        if (!read_f32_value(cursor, &angle)) return false;
        if (!read_f32_value(cursor, &spread)) return false;
        if (!read_f32_value(cursor, &start_size)) return false;
        if (!read_f32_value(cursor, &end_size)) return false;
        if (!read_f32_value(cursor, &speed)) return false;
        if (!read_f32_value(cursor, &rate)) return false;
        if (!read_u8_value(cursor, &outer_r)) return false;
        if (!read_u8_value(cursor, &outer_g)) return false;
        if (!read_u8_value(cursor, &outer_b)) return false;
        if (!read_u8_value(cursor, &inner_r)) return false;
        if (!read_u8_value(cursor, &inner_g)) return false;
        if (!read_u8_value(cursor, &inner_b)) return false;
        if (!read_u8_value(cursor, &loop)) return false;

        outer_color.r = outer_r;
        outer_color.g = outer_g;
        outer_color.b = outer_b;
        outer_color.a = 255;
        inner_color.r = inner_r;
        inner_color.g = inner_g;
        inner_color.b = inner_b;
        inner_color.a = 255;

        ParticleComponent* particle = ParticleComponent_get(entity);
        if (!particle) {
            particle = ParticleComponent_add(entity, angle, spread,
                start_size, end_size, speed, rate, outer_color, inner_color);
        }

        particle->angle = angle;
        particle->spread = spread;
        particle->start_size = start_size;
        particle->end_size = end_size;
        particle->speed = speed;
        particle->rate = rate;
        particle->outer_color = outer_color;
        particle->inner_color = inner_color;
        particle->loop = loop;
        if (loop) {
            particle->enabled = true;
        }

        return true;
    }

    float start_size;
    float width;
    float height;
    uint8_t loop;
    float rate;

    if (!read_f32_value(cursor, &start_size)) return false;
    if (!read_f32_value(cursor, &width)) return false;
    if (!read_f32_value(cursor, &height)) return false;
    if (!read_u8_value(cursor, &loop)) return false;
    if (!read_f32_value(cursor, &rate)) return false;

    ParticleComponent* particle = ParticleComponent_get(entity);
    if (!particle) {
        particle = ParticleComponent_add_type(entity, (ParticleType)type, start_size);
    }

    particle->start_size = start_size;
    particle->width = width;
    particle->height = height;
    particle->loop = loop;
    if (loop) {
        particle->enabled = true;
    }
    particle->rate = rate;
    return true;
}


static bool LightComponent_serialize_binary(BinaryWriteCursor* cursor, LightComponent* light) {
    if (!write_f32_value(cursor, light->range)) return false;
    if (!write_f32_value(cursor, light->angle)) return false;
    if (!write_u8_value(cursor, (uint8_t)light->color.r)) return false;
    if (!write_u8_value(cursor, (uint8_t)light->color.g)) return false;
    if (!write_u8_value(cursor, (uint8_t)light->color.b)) return false;
    if (!write_f32_value(cursor, light->max_brightness)) return false;
    if (!write_f32_value(cursor, light->speed)) return false;
    if (!write_u8_value(cursor, (uint8_t)light->enabled)) return false;
    if (!write_f32_value(cursor, light->flicker)) return false;
    if (!write_f32_value(cursor, light->brightness)) return false;
    return true;
}


static bool LightComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    float range;
    float angle;
    Color color;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;
    float max_brightness;
    float speed;
    uint8_t enabled;
    float flicker;
    float brightness;

    if (!read_f32_value(cursor, &range)) return false;
    if (!read_f32_value(cursor, &angle)) return false;
    if (!read_u8_value(cursor, &color_r)) return false;
    if (!read_u8_value(cursor, &color_g)) return false;
    if (!read_u8_value(cursor, &color_b)) return false;
    if (!read_f32_value(cursor, &max_brightness)) return false;
    if (!read_f32_value(cursor, &speed)) return false;
    if (!read_u8_value(cursor, &enabled)) return false;
    if (!read_f32_value(cursor, &flicker)) return false;
    if (!read_f32_value(cursor, &brightness)) return false;

    color.r = color_r;
    color.g = color_g;
    color.b = color_b;
    color.a = 255;

    LightComponent* light = LightComponent_get(entity);
    if (!light) {
        light = LightComponent_add(entity, range, angle, color, max_brightness, speed);
    }

    light->range = range;
    light->angle = angle;
    light->color = color;
    light->max_brightness = max_brightness;
    light->speed = speed;
    light->enabled = enabled;
    light->flicker = flicker;
    light->brightness = brightness;
    return true;
}


static bool SoundComponent_serialize_binary(BinaryWriteCursor* cursor, SoundComponent* sound) {
    if (!write_string_value(cursor, sound->hit_sound)) return false;
    if (!write_string_value(cursor, sound->loop_sound)) return false;
    return true;
}


static bool SoundComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    Filename hit_sound;
    Filename loop_sound;

    if (!read_string_value(cursor, hit_sound, sizeof(hit_sound))) return false;
    if (!read_string_value(cursor, loop_sound, sizeof(loop_sound))) return false;

    SoundComponent* sound = SoundComponent_get(entity);
    if (!sound) {
        sound = SoundComponent_add(entity, hit_sound);
    }

    strcpy(sound->hit_sound, hit_sound);
    strcpy(sound->loop_sound, loop_sound);
    return true;
}


static bool AmmoComponent_serialize_binary(BinaryWriteCursor* cursor, AmmoComponent* ammo) {
    if (!write_u8_value(cursor, (uint8_t)ammo->type)) return false;
    if (!write_i16_value(cursor, (int16_t)ammo->size)) return false;
    return true;
}


static bool AmmoComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    uint8_t type;
    int16_t size;

    if (!read_u8_value(cursor, &type)) return false;
    if (!read_i16_value(cursor, &size)) return false;

    AmmoComponent* ammo = AmmoComponent_get(entity);
    if (!ammo) {
        ammo = AmmoComponent_add(entity, (AmmoType)type);
    }

    ammo->type = (AmmoType)type;
    ammo->size = (int)size;
    return true;
}


static bool AnimationComponent_serialize_binary(BinaryWriteCursor* cursor, AnimationComponent* animation) {
    if (!write_i16_value(cursor, (int16_t)animation->frames)) return false;
    if (!write_f32_value(cursor, animation->framerate)) return false;
    if (!write_u8_value(cursor, (uint8_t)animation->play_once)) return false;
    if (!write_f32_value(cursor, animation->wind_factor)) return false;
    if (!write_i16_value(cursor, (int16_t)animation->current_frame)) return false;
    return true;
}


static bool AnimationComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    int16_t frames;
    float framerate;
    uint8_t play_once;
    float wind_factor;
    int16_t current_frame;

    if (!read_i16_value(cursor, &frames)) return false;
    if (!read_f32_value(cursor, &framerate)) return false;
    if (!read_u8_value(cursor, &play_once)) return false;
    if (!read_f32_value(cursor, &wind_factor)) return false;
    if (!read_i16_value(cursor, &current_frame)) return false;

    AnimationComponent* animation = AnimationComponent_get(entity);
    if (!animation) {
        animation = AnimationComponent_add(entity, (int)frames);
    }

    animation->frames = (int)frames;
    animation->framerate = framerate;
    animation->play_once = play_once;
    animation->wind_factor = wind_factor;
    animation->current_frame = (int)current_frame;
    return true;
}


static bool DoorComponent_serialize_binary(BinaryWriteCursor* cursor, DoorComponent* door) {
    if (!write_i16_value(cursor, (int16_t)door->price)) return false;
    if (!write_u8_value(cursor, (uint8_t)door->locked)) return false;
    if (!write_u8_value(cursor, (uint8_t)door->key)) return false;
    return true;
}


static bool DoorComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    int16_t price;
    uint8_t locked;
    uint8_t key;

    if (!read_i16_value(cursor, &price)) return false;
    if (!read_u8_value(cursor, &locked)) return false;
    if (!read_u8_value(cursor, &key)) return false;

    DoorComponent* door = DoorComponent_get(entity);
    if (!door) {
        door = DoorComponent_add(entity, (int)price);
    }

    door->price = (int)price;
    door->locked = locked;
    door->key = (int)key;
    return true;
}


static bool JointComponent_serialize_binary(BinaryWriteCursor* cursor, JointComponent* joint) {
    if (!write_i16_value(cursor, (int16_t)joint->parent)) return false;
    if (!write_f32_value(cursor, joint->min_length)) return false;
    if (!write_f32_value(cursor, joint->max_length)) return false;
    if (!write_f32_value(cursor, joint->strength)) return false;
    if (!write_f32_value(cursor, joint->max_angle)) return false;
    return true;
}


static bool JointComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    int16_t parent;
    float min_length;
    float max_length;
    float strength;
    float max_angle;

    if (!read_i16_value(cursor, &parent)) return false;
    if (!read_f32_value(cursor, &min_length)) return false;
    if (!read_f32_value(cursor, &max_length)) return false;
    if (!read_f32_value(cursor, &strength)) return false;
    if (!read_f32_value(cursor, &max_angle)) return false;

    JointComponent* joint = JointComponent_get(entity);
    if (!joint) {
        joint = JointComponent_add(entity, (int)parent, min_length, max_length, strength);
    }

    joint->parent = (int)parent;
    joint->min_length = min_length;
    joint->max_length = max_length;
    joint->strength = strength;
    joint->max_angle = max_angle;
    return true;
}


static bool VehicleComponent_serialize_binary(BinaryWriteCursor* cursor, VehicleComponent* vehicle) {
    if (!write_f32_value(cursor, vehicle->max_fuel)) return false;
    if (!write_u8_value(cursor, (uint8_t)vehicle->size)) return false;
    if (!write_f32_value(cursor, vehicle->acceleration)) return false;
    if (!write_f32_value(cursor, vehicle->max_speed)) return false;
    if (!write_f32_value(cursor, vehicle->turning)) return false;
    for (int i = 0; i < vehicle->size && i < 4; i++) {
        if (!write_f32_value(cursor, vehicle->seats[i].x)) return false;
        if (!write_f32_value(cursor, vehicle->seats[i].y)) return false;
    }
    if (!write_f32_value(cursor, vehicle->fuel)) return false;
    if (!write_u8_value(cursor, vehicle->on_road ? 1 : 0)) return false;
    for (int i = 0; i < 4; i++) {
        if (!write_i16_value(cursor, (int16_t)vehicle->riders[i])) return false;
    }
    return true;
}


static bool VehicleComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    float max_fuel;
    uint8_t size;
    float acceleration;
    float max_speed;
    float turning;
    Vector2f seats[4] = { 0 };
    float fuel;
    uint8_t on_road;
    int16_t riders[4];

    if (!read_f32_value(cursor, &max_fuel)) return false;
    if (!read_u8_value(cursor, &size)) return false;
    if (!read_f32_value(cursor, &acceleration)) return false;
    if (!read_f32_value(cursor, &max_speed)) return false;
    if (!read_f32_value(cursor, &turning)) return false;

    int seat_count = (size < 4) ? size : 4;
    for (int i = 0; i < seat_count; i++) {
        if (!read_f32_value(cursor, &seats[i].x)) return false;
        if (!read_f32_value(cursor, &seats[i].y)) return false;
    }

    if (!read_f32_value(cursor, &fuel)) return false;
    if (!read_u8_value(cursor, &on_road)) return false;
    for (int i = 0; i < 4; i++) {
        if (!read_i16_value(cursor, &riders[i])) return false;
    }

    VehicleComponent* vehicle = VehicleComponent_get(entity);
    if (!vehicle) {
        vehicle = VehicleComponent_add(entity, max_fuel);
    }

    vehicle->max_fuel = max_fuel;
    vehicle->size = (int)size;
    vehicle->acceleration = acceleration;
    vehicle->max_speed = max_speed;
    vehicle->turning = turning;
    for (int i = 0; i < seat_count; i++) {
        vehicle->seats[i] = seats[i];
    }
    vehicle->fuel = fuel;
    vehicle->on_road = on_road != 0;
    for (int i = 0; i < 4; i++) {
        vehicle->riders[i] = (int)riders[i];
    }
    return true;
}


static bool TextComponent_serialize_binary(BinaryWriteCursor* cursor, TextComponent* text) {
    if (!write_string_value(cursor, text->source_string)) return false;
    if (!write_i16_value(cursor, (int16_t)text->size)) return false;
    if (!write_u8_value(cursor, (uint8_t)text->color.r)) return false;
    if (!write_u8_value(cursor, (uint8_t)text->color.g)) return false;
    if (!write_u8_value(cursor, (uint8_t)text->color.b)) return false;
    return true;
}


static bool TextComponent_deserialize_binary(BinaryReadCursor* cursor, int entity) {
    String source_string;
    int16_t size;
    Color color;
    uint8_t color_r;
    uint8_t color_g;
    uint8_t color_b;

    if (!read_string_value(cursor, source_string, sizeof(source_string))) return false;
    if (!read_i16_value(cursor, &size)) return false;
    if (!read_u8_value(cursor, &color_r)) return false;
    if (!read_u8_value(cursor, &color_g)) return false;
    if (!read_u8_value(cursor, &color_b)) return false;

    color.r = color_r;
    color.g = color_g;
    color.b = color_b;
    color.a = 255;

    TextComponent* text = TextComponent_get(entity);
    if (!text) {
        TextComponent_add(entity, source_string, (int)size, color);
    } else {
        strcpy(text->source_string, source_string);
        text->size = (int)size;
        text->color = color;
    }
    return true;
}


int binary_serialize_entity(uint8_t* buf, int buf_size, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (!coord) {
        return 0;
    }

    ImageComponent* image = ImageComponent_get(entity);
    PhysicsComponent* physics = PhysicsComponent_get(entity);
    ColliderComponent* collider = ColliderComponent_get(entity);
    PlayerComponent* player = PlayerComponent_get(entity);
    EnemyComponent* enemy = EnemyComponent_get(entity);
    HealthComponent* health = HealthComponent_get(entity);
    WeaponComponent* weapon = WeaponComponent_get(entity);
    ItemComponent* item = ItemComponent_get(entity);
    WaypointComponent* waypoint = WaypointComponent_get(entity);
    ParticleComponent* particle = ParticleComponent_get(entity);
    LightComponent* light = LightComponent_get(entity);
    SoundComponent* sound = SoundComponent_get(entity);
    AmmoComponent* ammo = AmmoComponent_get(entity);
    AnimationComponent* animation = AnimationComponent_get(entity);
    DoorComponent* door = DoorComponent_get(entity);
    JointComponent* joint = JointComponent_get(entity);
    VehicleComponent* vehicle = VehicleComponent_get(entity);
    TextComponent* text = TextComponent_get(entity);

    uint32_t flags = 0;
    if (image) flags |= BFLAG_HAS_IMAGE;
    if (physics) flags |= BFLAG_HAS_PHYSICS;
    if (collider) flags |= BFLAG_HAS_COLLIDER;
    if (player) flags |= BFLAG_HAS_PLAYER;
    if (enemy) flags |= BFLAG_HAS_ENEMY;
    if (health) flags |= BFLAG_HAS_HEALTH;
    if (weapon) flags |= BFLAG_HAS_WEAPON;
    if (item) flags |= BFLAG_HAS_ITEM;
    if (waypoint) flags |= BFLAG_HAS_WAYPOINT;
    if (particle) flags |= BFLAG_HAS_PARTICLE;
    if (light) flags |= BFLAG_HAS_LIGHT;
    if (sound) flags |= BFLAG_HAS_SOUND;
    if (ammo) flags |= BFLAG_HAS_AMMO;
    if (animation) flags |= BFLAG_HAS_ANIMATION;
    if (door) flags |= BFLAG_HAS_DOOR;
    if (joint) flags |= BFLAG_HAS_JOINT;
    if (vehicle) flags |= BFLAG_HAS_VEHICLE;
    if (text) flags |= BFLAG_HAS_TEXT;

    BinaryWriteCursor cursor = { buf, buf_size };

    if (!CoordinateComponent_serialize_binary(&cursor, coord)) return 0;
    if (!write_u32_value(&cursor, flags)) return 0;
    if (image && !ImageComponent_serialize_binary(&cursor, image)) return 0;
    if (physics && !PhysicsComponent_serialize_binary(&cursor, physics)) return 0;
    if (collider && !ColliderComponent_serialize_binary(&cursor, collider)) return 0;
    if (player && !PlayerComponent_serialize_binary(&cursor, player)) return 0;
    if (enemy && !EnemyComponent_serialize_binary(&cursor, enemy)) return 0;
    if (health && !HealthComponent_serialize_binary(&cursor, health)) return 0;
    if (weapon && !WeaponComponent_serialize_binary(&cursor, weapon)) return 0;
    if (item && !ItemComponent_serialize_binary(&cursor, item)) return 0;
    if (waypoint && !WaypointComponent_serialize_binary(&cursor, waypoint)) return 0;
    if (particle && !ParticleComponent_serialize_binary(&cursor, particle)) return 0;
    if (light && !LightComponent_serialize_binary(&cursor, light)) return 0;
    if (sound && !SoundComponent_serialize_binary(&cursor, sound)) return 0;
    if (ammo && !AmmoComponent_serialize_binary(&cursor, ammo)) return 0;
    if (animation && !AnimationComponent_serialize_binary(&cursor, animation)) return 0;
    if (door && !DoorComponent_serialize_binary(&cursor, door)) return 0;
    if (joint && !JointComponent_serialize_binary(&cursor, joint)) return 0;
    if (vehicle && !VehicleComponent_serialize_binary(&cursor, vehicle)) return 0;
    if (text && !TextComponent_serialize_binary(&cursor, text)) return 0;

    return (int)(cursor.ptr - buf);
}


int binary_deserialize_entity(const uint8_t* buf, int buf_size, int entity, bool smooth) {
    BinaryReadCursor cursor = { buf, buf + buf_size };
    uint32_t flags;

    if (!CoordinateComponent_deserialize_binary(&cursor, entity, &smooth)) return 0;
    if (!read_u32_value(&cursor, &flags)) return 0;
    if ((flags & BFLAG_HAS_IMAGE) && !ImageComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_PHYSICS) && !PhysicsComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_COLLIDER) && !ColliderComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_PLAYER) && !PlayerComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_ENEMY) && !EnemyComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_HEALTH) && !HealthComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_WEAPON) && !WeaponComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_ITEM) && !ItemComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_WAYPOINT) && !WaypointComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_PARTICLE) && !ParticleComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_LIGHT) && !LightComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_SOUND) && !SoundComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_AMMO) && !AmmoComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_ANIMATION) && !AnimationComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_DOOR) && !DoorComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_JOINT) && !JointComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_VEHICLE) && !VehicleComponent_deserialize_binary(&cursor, entity)) return 0;
    if ((flags & BFLAG_HAS_TEXT) && !TextComponent_deserialize_binary(&cursor, entity)) return 0;

    return (int)(cursor.ptr - buf);
}
