#include <stdio.h>
#include <string.h>

#include "netgame.h"
#include "network.h"
#include "component.h"
#include "game.h"
#include "enemy.h"
#include "weapon.h"
#include "item.h"
#include "grid.h"
#include "image.h"

#ifndef __EMSCRIPTEN__

bool net_entity_seen[MAX_ENTITIES];
int net_map_max_entity = 0;
int net_host_to_local[MAX_ENTITIES];
int net_local_to_host[MAX_ENTITIES];


int net_resolve_id(int host_id) {
    if (host_id < 0 || host_id >= MAX_ENTITIES) return host_id;
    return (net_host_to_local[host_id] >= 0) ? net_host_to_local[host_id] : host_id;
}


void net_clear_id_map(void) {
    memset(net_host_to_local, -1, sizeof(net_host_to_local));
    memset(net_local_to_host, -1, sizeof(net_local_to_host));
}


bool netgame_is_dynamic(int entity) {
    if (PhysicsComponent_get(entity)) return true;
    if (HealthComponent_get(entity)) return true;
    if (PlayerComponent_get(entity)) return true;
    if (EnemyComponent_get(entity)) return true;
    if (WeaponComponent_get(entity)) return true;
    if (VehicleComponent_get(entity)) return true;
    if (DoorComponent_get(entity)) return true;
    if (ItemComponent_get(entity)) return true;
    if (AmmoComponent_get(entity)) return true;
    return false;
}


// Write a single entity's variable-length snapshot into buf.
// Returns number of bytes written, or 0 if not enough space.
static int build_entity_snapshot(uint8_t* buf, int remaining, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(entity);
    PhysicsComponent* phys = PhysicsComponent_get(entity);
    HealthComponent* health = HealthComponent_get(entity);
    ImageComponent* image = ImageComponent_get(entity);
    PlayerComponent* player = PlayerComponent_get(entity);
    EnemyComponent* enemy = EnemyComponent_get(entity);
    AnimationComponent* anim = AnimationComponent_get(entity);
    WeaponComponent* weapon = WeaponComponent_get(entity);
    DoorComponent* door = DoorComponent_get(entity);
    ItemComponent* item = ItemComponent_get(entity);
    AmmoComponent* ammo_comp = AmmoComponent_get(entity);
    LightComponent* light = LightComponent_get(entity);
    VehicleComponent* vehicle = VehicleComponent_get(entity);

    // Determine which component sections to include
    uint16_t flags = 0;
    if (phys)      flags |= SNAP_HAS_PHYSICS;
    if (health)    flags |= SNAP_HAS_HEALTH;
    if (player)    flags |= SNAP_HAS_PLAYER;
    if (enemy)     flags |= SNAP_HAS_ENEMY;
    if (weapon)    flags |= SNAP_HAS_WEAPON;
    if (door)      flags |= SNAP_HAS_DOOR;
    if (item)      flags |= SNAP_HAS_ITEM;
    if (ammo_comp) flags |= SNAP_HAS_AMMO;
    if (light)     flags |= SNAP_HAS_LIGHT;
    if (image)     flags |= SNAP_HAS_IMAGE;
    if (anim)      flags |= SNAP_HAS_ANIM;
    if (vehicle)   flags |= SNAP_HAS_VEHICLE;

    // Calculate total size needed
    int size = (int)sizeof(SnapBase);
    if (flags & SNAP_HAS_PHYSICS) size += (int)sizeof(SnapPhysics);
    if (flags & SNAP_HAS_HEALTH)  size += (int)sizeof(SnapHealth);
    if (flags & SNAP_HAS_PLAYER)  size += (int)sizeof(SnapPlayer);
    if (flags & SNAP_HAS_ENEMY)   size += (int)sizeof(SnapEnemy);
    if (flags & SNAP_HAS_WEAPON)  size += (int)sizeof(SnapWeapon);
    if (flags & SNAP_HAS_DOOR)    size += (int)sizeof(SnapDoor);
    if (flags & SNAP_HAS_ITEM)    size += (int)sizeof(SnapItem);
    if (flags & SNAP_HAS_AMMO)    size += (int)sizeof(SnapAmmo);
    if (flags & SNAP_HAS_LIGHT)   size += (int)sizeof(SnapLight);
    if (flags & SNAP_HAS_IMAGE)   size += (int)sizeof(SnapImage);
    if (flags & SNAP_HAS_ANIM)    size += (int)sizeof(SnapAnim);
    if (flags & SNAP_HAS_VEHICLE) size += (int)sizeof(SnapVehicle);

    if (size > remaining) return 0;

    uint8_t* ptr = buf;

    // Write base
    {
        SnapBase base;
        base.entity_id = (uint16_t)entity;
        base.net_type = coord->net_type;
        base.comp_flags = flags;
        base.pos_x = coord->position.x;
        base.pos_y = coord->position.y;
        base.angle = coord->angle;
        base.parent = (int16_t)coord->parent;
        base.scale_x = coord->scale.x;
        base.scale_y = coord->scale.y;
        memcpy(ptr, &base, sizeof(SnapBase));
        ptr += sizeof(SnapBase);
    }

    // Write optional sections in flag order
    if (flags & SNAP_HAS_PHYSICS) {
        SnapPhysics sp;
        sp.vel_x = phys->velocity.x;
        sp.vel_y = phys->velocity.y;
        memcpy(ptr, &sp, sizeof(SnapPhysics));
        ptr += sizeof(SnapPhysics);
    }

    if (flags & SNAP_HAS_HEALTH) {
        SnapHealth sh;
        sh.health = (int16_t)health->health;
        sh.max_health = (int16_t)health->max_health;
        sh.dead = health->dead ? 1 : 0;
        sh.status_type = (uint8_t)health->status.type;
        sh._pad = 0;
        memcpy(ptr, &sh, sizeof(SnapHealth));
        ptr += sizeof(SnapHealth);
    }

    if (flags & SNAP_HAS_PLAYER) {
        SnapPlayer sp;
        sp.state = (uint8_t)player->state;
        sp.money = (int32_t)player->money;
        sp.item = (uint8_t)player->item;
        for (int j = 0; j < 4; j++) sp.inventory[j] = (int16_t)player->inventory[j];
        for (int j = 0; j < 4; j++) sp.ammo[j] = (int16_t)player->ammo[j];
        for (int j = 0; j < 3; j++) sp.keys[j] = (uint8_t)player->keys[j];
        sp.arms = (int16_t)player->arms;
        sp.vehicle = (int16_t)player->vehicle;
        sp.target = (int16_t)player->target;
        sp.use_timer = player->use_timer;
        sp.grabbed_item = (int16_t)player->grabbed_item;
        sp.won = player->won ? 1 : 0;
        sp.money_timer = player->money_timer;
        sp.money_increment = (int32_t)player->money_increment;
        memcpy(ptr, &sp, sizeof(SnapPlayer));
        ptr += sizeof(SnapPlayer);
    }

    if (flags & SNAP_HAS_ENEMY) {
        SnapEnemy se;
        se.state = (uint8_t)enemy->state;
        se.weapon = (int16_t)enemy->weapon;
        se.boss = enemy->boss ? 1 : 0;
        se.bounty = (int32_t)enemy->bounty;
        se.target = (int16_t)enemy->target;
        memcpy(ptr, &se, sizeof(SnapEnemy));
        ptr += sizeof(SnapEnemy);
    }

    if (flags & SNAP_HAS_WEAPON) {
        SnapWeapon sw;
        sw.recoil = weapon->recoil;
        sw.spread = weapon->spread;
        sw.cooldown = weapon->cooldown;
        sw.magazine = (int16_t)weapon->magazine;
        sw.reloading = weapon->reloading ? 1 : 0;
        sw.ammo_type = (uint8_t)weapon->ammo_type;
        memcpy(ptr, &sw, sizeof(SnapWeapon));
        ptr += sizeof(SnapWeapon);
    }

    if (flags & SNAP_HAS_DOOR) {
        SnapDoor sd;
        sd.locked = door->locked ? 1 : 0;
        sd.price = (int16_t)door->price;
        sd.key = (uint8_t)door->key;
        sd._pad[0] = 0;
        sd._pad[1] = 0;
        memcpy(ptr, &sd, sizeof(SnapDoor));
        ptr += sizeof(SnapDoor);
    }

    if (flags & SNAP_HAS_ITEM) {
        SnapItem si;
        si.price = (int16_t)item->price;
        for (int j = 0; j < 5; j++) si.attachments[j] = (int16_t)item->attachments[j];
        si.size = (uint8_t)item->size;
        si.type = (uint8_t)item->type;
        memcpy(ptr, &si, sizeof(SnapItem));
        ptr += sizeof(SnapItem);
    }

    if (flags & SNAP_HAS_AMMO) {
        SnapAmmo sa;
        sa.type = (uint8_t)ammo_comp->type;
        sa.size = (int16_t)ammo_comp->size;
        memcpy(ptr, &sa, sizeof(SnapAmmo));
        ptr += sizeof(SnapAmmo);
    }

    if (flags & SNAP_HAS_LIGHT) {
        SnapLight sl;
        sl.enabled = light->enabled ? 1 : 0;
        sl.brightness = light->brightness;
        memcpy(ptr, &sl, sizeof(SnapLight));
        ptr += sizeof(SnapLight);
    }

    if (flags & SNAP_HAS_IMAGE) {
        SnapImage sim;
        sim.alpha = image->alpha;
        sim.layer = (uint8_t)image->layer;
        memcpy(ptr, &sim, sizeof(SnapImage));
        ptr += sizeof(SnapImage);
    }

    if (flags & SNAP_HAS_ANIM) {
        SnapAnim san;
        san.current_frame = (uint8_t)anim->current_frame;
        memcpy(ptr, &san, sizeof(SnapAnim));
        ptr += sizeof(SnapAnim);
    }

    if (flags & SNAP_HAS_VEHICLE) {
        SnapVehicle sv;
        sv.fuel = vehicle->fuel;
        sv.on_road = vehicle->on_road ? 1 : 0;
        for (int j = 0; j < 4; j++) sv.riders[j] = (int16_t)vehicle->riders[j];
        sv.size = (uint8_t)vehicle->size;
        memcpy(ptr, &sv, sizeof(SnapVehicle));
        ptr += sizeof(SnapVehicle);
    }

    return (int)(ptr - buf);
}


int netgame_build_snapshot(uint8_t* buf, int buf_size, uint32_t tick) {
    SnapshotPacket* pkt = (SnapshotPacket*)buf;
    pkt->header.type = PACKET_SNAPSHOT;
    pkt->header.tick = tick;
    pkt->entity_count = 0;

    uint8_t* data = buf + sizeof(SnapshotPacket);
    int remaining = buf_size - (int)sizeof(SnapshotPacket);

    for (int i = 0; i < game_data->components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;

        // Skip cameras, widgets, menu entities
        if (CameraComponent_get(i)) continue;
        if (WidgetComponent_get(i)) continue;

        if (!netgame_is_dynamic(i)) continue;

        int written = build_entity_snapshot(data, remaining, i);
        if (written == 0) {
            LOG_WARNING("Snapshot buffer full at %d entities", pkt->entity_count);
            break;
        }

        data += written;
        remaining -= written;
        pkt->entity_count++;
    }

    int total_size = (int)(data - buf);
    pkt->header.size = (uint16_t)total_size;

    return total_size;
}


int netgame_create_entity_from_snapshot(const SnapBase* base) {
    // Runtime-created entity: use factory function based on net_type,
    // then overwrite state from snapshot.
    // For map entities we should never get here (they already exist).
    Vector2f pos = { base->pos_x, base->pos_y };
    int entity = -1;

    switch ((NetEntityType)base->net_type) {
        case NET_ENTITY_ZOMBIE:
            entity = create_zombie(pos, base->angle);
            break;
        case NET_ENTITY_FARMER:
            entity = create_farmer(pos, base->angle);
            break;
        case NET_ENTITY_PRIEST:
            entity = create_priest(pos, base->angle);
            break;
        case NET_ENTITY_BIG_BOY:
            entity = create_big_boy(pos, base->angle);
            break;
        case NET_ENTITY_BOSS:
            entity = create_boss(pos, base->angle);
            break;
        case NET_ENTITY_PISTOL:
            entity = create_pistol(pos);
            break;
        case NET_ENTITY_SHOTGUN:
            entity = create_shotgun(pos);
            break;
        case NET_ENTITY_SAWED_OFF:
            entity = create_sawed_off(pos);
            break;
        case NET_ENTITY_RIFLE:
            entity = create_rifle(pos);
            break;
        case NET_ENTITY_ASSAULT_RIFLE:
            entity = create_assault_rifle(pos);
            break;
        case NET_ENTITY_SMG:
            entity = create_smg(pos);
            break;
        case NET_ENTITY_AXE:
            entity = create_axe(pos);
            break;
        case NET_ENTITY_SWORD:
            entity = create_sword(pos);
            break;
        case NET_ENTITY_BANDAGE:
            entity = create_bandage(pos);
            break;
        case NET_ENTITY_AMMO_PISTOL:
            entity = create_ammo(pos, AMMO_PISTOL);
            break;
        case NET_ENTITY_AMMO_RIFLE:
            entity = create_ammo(pos, AMMO_RIFLE);
            break;
        case NET_ENTITY_AMMO_SHOTGUN:
            entity = create_ammo(pos, AMMO_SHOTGUN);
            break;
        case NET_ENTITY_ENERGY:
            create_energy(pos, (Vector2f){ 0.0f, 0.0f });
            // create_energy doesn't return entity id, find it
            for (int i = game_data->components->entities - 1; i >= 0; i--) {
                CoordinateComponent* c = CoordinateComponent_get(i);
                if (c) { entity = i; break; }
            }
            break;
        case NET_ENTITY_FLAME:
            entity = create_flame(pos, (Vector2f){ 0.0f, 0.0f }, -1);
            break;
        case NET_ENTITY_SPLASH:
            entity = create_splash(pos, (Vector2f){ 0.0f, 0.0f });
            break;
        case NET_ENTITY_FREEZE:
            entity = create_freeze(pos, (Vector2f){ 0.0f, 0.0f });
            break;
        case NET_ENTITY_ROPE:
            // Rope is complex (multi-entity chain). Skip for now.
            break;
        case NET_ENTITY_DECAL:
        case NET_ENTITY_NONE:
        default:
            break;
    }

    return entity;
}


// Read a single variable-length entity snapshot from buf and apply it.
// Returns number of bytes consumed, or 0 on error.
static int apply_entity_snapshot(const uint8_t* buf, int remaining) {
    if (remaining < (int)sizeof(SnapBase)) return 0;

    const SnapBase* base = (const SnapBase*)buf;
    uint16_t flags = base->comp_flags;
    int host_id = (int)base->entity_id;

    // Calculate expected size
    int size = (int)sizeof(SnapBase);
    if (flags & SNAP_HAS_PHYSICS) size += (int)sizeof(SnapPhysics);
    if (flags & SNAP_HAS_HEALTH)  size += (int)sizeof(SnapHealth);
    if (flags & SNAP_HAS_PLAYER)  size += (int)sizeof(SnapPlayer);
    if (flags & SNAP_HAS_ENEMY)   size += (int)sizeof(SnapEnemy);
    if (flags & SNAP_HAS_WEAPON)  size += (int)sizeof(SnapWeapon);
    if (flags & SNAP_HAS_DOOR)    size += (int)sizeof(SnapDoor);
    if (flags & SNAP_HAS_ITEM)    size += (int)sizeof(SnapItem);
    if (flags & SNAP_HAS_AMMO)    size += (int)sizeof(SnapAmmo);
    if (flags & SNAP_HAS_LIGHT)   size += (int)sizeof(SnapLight);
    if (flags & SNAP_HAS_IMAGE)   size += (int)sizeof(SnapImage);
    if (flags & SNAP_HAS_ANIM)    size += (int)sizeof(SnapAnim);
    if (flags & SNAP_HAS_VEHICLE) size += (int)sizeof(SnapVehicle);

    if (size > remaining) return 0;

    // Resolve host entity ID to local entity ID
    int entity = net_resolve_id(host_id);

    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (!coord) {
        // Entity doesn't exist locally.
        // If it's a map entity (net_type == NONE), it was destroyed on host. Ignore.
        if (base->net_type == NET_ENTITY_NONE) return size;

        // Runtime entity: create it using the factory
        int created = netgame_create_entity_from_snapshot(base);
        if (created == -1) return size;

        // If created != host_id, record the ID mapping
        if (created != host_id) {
            LOG_WARNING("Entity ID mismatch: host=%d local=%d (net_type=%d)",
                        host_id, created, base->net_type);
            if (host_id >= 0 && host_id < MAX_ENTITIES &&
                created >= 0 && created < MAX_ENTITIES) {
                net_host_to_local[host_id] = created;
                net_local_to_host[created] = host_id;
            }
        }

        coord = CoordinateComponent_get(created);
        if (!coord) return size;
        entity = created;
    }

    // Mark as seen (using local entity ID)
    net_entity_seen[entity] = true;

    // Apply base fields
    coord->position.x = base->pos_x;
    coord->position.y = base->pos_y;
    coord->angle = base->angle;
    coord->parent = (base->parent >= 0) ? net_resolve_id((int)base->parent) : (int)base->parent;
    coord->scale.x = base->scale_x;
    coord->scale.y = base->scale_y;

    // Walk through optional sections
    const uint8_t* ptr = buf + sizeof(SnapBase);

    if (flags & SNAP_HAS_PHYSICS) {
        const SnapPhysics* sp = (const SnapPhysics*)ptr;
        PhysicsComponent* phys = PhysicsComponent_get(entity);
        if (phys) {
            phys->velocity.x = sp->vel_x;
            phys->velocity.y = sp->vel_y;
        }
        ptr += sizeof(SnapPhysics);
    }

    if (flags & SNAP_HAS_HEALTH) {
        const SnapHealth* sh = (const SnapHealth*)ptr;
        HealthComponent* health = HealthComponent_get(entity);
        if (health) {
            health->health = (int)sh->health;
            health->max_health = (int)sh->max_health;
            health->dead = sh->dead != 0;
            health->status.type = (StatusEffect)sh->status_type;
        }
        ptr += sizeof(SnapHealth);
    }

    if (flags & SNAP_HAS_PLAYER) {
        const SnapPlayer* sp = (const SnapPlayer*)ptr;
        PlayerComponent* player = PlayerComponent_get(entity);
        if (player) {
            player->state = (PlayerState)sp->state;
            player->money = (int)sp->money;
            player->item = (int)sp->item;
            for (int j = 0; j < 4; j++) {
                int id = (int)sp->inventory[j];
                player->inventory[j] = (id >= 0) ? net_resolve_id(id) : id;
            }
            for (int j = 0; j < 4; j++) {
                int id = (int)sp->ammo[j];
                player->ammo[j] = (id >= 0) ? net_resolve_id(id) : id;
            }
            for (int j = 0; j < 3; j++) player->keys[j] = (int)sp->keys[j];
            player->arms = (sp->arms >= 0) ? net_resolve_id((int)sp->arms) : (int)sp->arms;
            player->vehicle = (sp->vehicle >= 0) ? net_resolve_id((int)sp->vehicle) : (int)sp->vehicle;
            player->target = (sp->target >= 0) ? net_resolve_id((int)sp->target) : (int)sp->target;
            player->use_timer = sp->use_timer;
            player->grabbed_item = (sp->grabbed_item >= 0) ? net_resolve_id((int)sp->grabbed_item) : (int)sp->grabbed_item;
            player->won = sp->won != 0;
            player->money_timer = sp->money_timer;
            player->money_increment = (int)sp->money_increment;
        }
        ptr += sizeof(SnapPlayer);
    }

    if (flags & SNAP_HAS_ENEMY) {
        const SnapEnemy* se = (const SnapEnemy*)ptr;
        EnemyComponent* enemy = EnemyComponent_get(entity);
        if (enemy) {
            enemy->state = (EnemyState)se->state;
            enemy->weapon = (se->weapon >= 0) ? net_resolve_id((int)se->weapon) : (int)se->weapon;
            enemy->boss = se->boss != 0;
            enemy->bounty = (int)se->bounty;
            enemy->target = (se->target >= 0) ? net_resolve_id((int)se->target) : (int)se->target;
        }
        ptr += sizeof(SnapEnemy);
    }

    if (flags & SNAP_HAS_WEAPON) {
        const SnapWeapon* sw = (const SnapWeapon*)ptr;
        WeaponComponent* weapon = WeaponComponent_get(entity);
        if (weapon) {
            weapon->recoil = sw->recoil;
            weapon->spread = sw->spread;
            weapon->cooldown = sw->cooldown;
            weapon->magazine = (int)sw->magazine;
            weapon->reloading = sw->reloading != 0;
            weapon->ammo_type = (AmmoType)sw->ammo_type;
        }
        ptr += sizeof(SnapWeapon);
    }

    if (flags & SNAP_HAS_DOOR) {
        const SnapDoor* sd = (const SnapDoor*)ptr;
        DoorComponent* door = DoorComponent_get(entity);
        if (door) {
            door->locked = sd->locked != 0;
            door->price = (int)sd->price;
            door->key = (int)sd->key;
        }
        ptr += sizeof(SnapDoor);
    }

    if (flags & SNAP_HAS_ITEM) {
        const SnapItem* si = (const SnapItem*)ptr;
        ItemComponent* item_c = ItemComponent_get(entity);
        if (item_c) {
            item_c->price = (int)si->price;
            for (int j = 0; j < 5; j++) {
                int id = (int)si->attachments[j];
                item_c->attachments[j] = (id >= 0) ? net_resolve_id(id) : id;
            }
            item_c->size = (int)si->size;
            item_c->type = (ItemType)si->type;
        }
        ptr += sizeof(SnapItem);
    }

    if (flags & SNAP_HAS_AMMO) {
        const SnapAmmo* sa = (const SnapAmmo*)ptr;
        AmmoComponent* ammo_c = AmmoComponent_get(entity);
        if (ammo_c) {
            ammo_c->type = (AmmoType)sa->type;
            ammo_c->size = (int)sa->size;
        }
        ptr += sizeof(SnapAmmo);
    }

    if (flags & SNAP_HAS_LIGHT) {
        const SnapLight* sl = (const SnapLight*)ptr;
        LightComponent* light = LightComponent_get(entity);
        if (light) {
            light->enabled = sl->enabled != 0;
            light->brightness = sl->brightness;
        }
        ptr += sizeof(SnapLight);
    }

    if (flags & SNAP_HAS_IMAGE) {
        const SnapImage* sim = (const SnapImage*)ptr;
        ImageComponent* image = ImageComponent_get(entity);
        if (image) {
            image->alpha = sim->alpha;
            image->layer = (Layer)sim->layer;
        }
        ptr += sizeof(SnapImage);
    }

    if (flags & SNAP_HAS_ANIM) {
        const SnapAnim* san = (const SnapAnim*)ptr;
        AnimationComponent* anim = AnimationComponent_get(entity);
        if (anim) {
            anim->current_frame = (int)san->current_frame;
        }
        ptr += sizeof(SnapAnim);
    }

    if (flags & SNAP_HAS_VEHICLE) {
        const SnapVehicle* sv = (const SnapVehicle*)ptr;
        VehicleComponent* veh = VehicleComponent_get(entity);
        if (veh) {
            veh->fuel = sv->fuel;
            veh->on_road = sv->on_road != 0;
            for (int j = 0; j < 4; j++) {
                int id = (int)sv->riders[j];
                veh->riders[j] = (id >= 0) ? net_resolve_id(id) : id;
            }
            veh->size = (int)sv->size;
        }
        ptr += sizeof(SnapVehicle);
    }

    return size;
}


void netgame_apply_snapshot(const uint8_t* buf, int size) {
    if (size < (int)sizeof(SnapshotPacket)) return;

    const SnapshotPacket* pkt = (const SnapshotPacket*)buf;
    if (pkt->header.type != PACKET_SNAPSHOT) return;

    const uint8_t* data = buf + sizeof(SnapshotPacket);
    int remaining = size - (int)sizeof(SnapshotPacket);

    // Track which LOCAL entity IDs are in this snapshot
    bool in_snapshot[MAX_ENTITIES];
    memset(in_snapshot, 0, sizeof(in_snapshot));

    // Apply each entity snapshot
    for (int i = 0; i < pkt->entity_count; i++) {
        if (remaining < (int)sizeof(SnapBase)) {
            LOG_WARNING("Snapshot truncated at entity %d/%d", i, pkt->entity_count);
            break;
        }

        const SnapBase* base = (const SnapBase*)data;
        int host_id = (int)base->entity_id;
        int local_id = net_resolve_id(host_id);
        if (local_id >= 0 && local_id < MAX_ENTITIES) {
            in_snapshot[local_id] = true;
        }

        int consumed = apply_entity_snapshot(data, remaining);
        if (consumed == 0) {
            LOG_WARNING("Failed to apply entity snapshot %d/%d", i, pkt->entity_count);
            break;
        }

        // After apply, the entity might have been created at a new local ID
        // (if it was a new runtime entity with an ID mismatch).
        // Re-resolve to make sure in_snapshot tracks the correct local ID.
        local_id = net_resolve_id(host_id);
        if (local_id >= 0 && local_id < MAX_ENTITIES) {
            in_snapshot[local_id] = true;
        }

        data += consumed;
        remaining -= consumed;
    }

    // Implicit destroy: entities that were seen before but aren't in this snapshot
    for (int i = 0; i < game_data->components->entities; i++) {
        if (net_entity_seen[i] && !in_snapshot[i]) {
            // This entity was in a previous snapshot but no longer exists on host
            CoordinateComponent* coord = CoordinateComponent_get(i);
            if (coord) {
                // Skip cameras, widgets
                if (CameraComponent_get(i)) continue;
                if (WidgetComponent_get(i)) continue;

                // Only destroy dynamic entities
                if (netgame_is_dynamic(i)) {
                    if (ColliderComponent_get(i)) {
                        clear_grid(i);
                    }
                    destroy_entity_recursive(i);
                }
            }
            net_entity_seen[i] = false;

            // Clean up ID mapping if this entity was mapped
            int host_id = net_local_to_host[i];
            if (host_id >= 0 && host_id < MAX_ENTITIES) {
                net_host_to_local[host_id] = -1;
                net_local_to_host[i] = -1;
            }
        }
    }
}


void netgame_pack_input(InputPacket* pkt, int player_entity, uint8_t player_slot, uint32_t tick) {
    memset(pkt, 0, sizeof(InputPacket));
    pkt->header.type = PACKET_INPUT;
    pkt->header.tick = tick;
    pkt->header.size = sizeof(InputPacket);
    pkt->player_slot = player_slot;

    PlayerComponent* player = PlayerComponent_get(player_entity);
    if (!player) return;

    Controller* ctrl = &player->controller;

    pkt->left_stick_x = ctrl->left_stick.x;
    pkt->left_stick_y = ctrl->left_stick.y;
    pkt->right_stick_x = ctrl->right_stick.x;
    pkt->right_stick_y = ctrl->right_stick.y;
    pkt->left_trigger = ctrl->left_trigger;
    pkt->right_trigger = ctrl->right_trigger;

    uint16_t down = 0, pressed = 0, released = 0;
    for (int i = 0; i < 12; i++) {
        if (ctrl->buttons_down[i])    down    |= (1 << i);
        if (ctrl->buttons_pressed[i]) pressed |= (1 << i);
        if (ctrl->buttons_released[i]) released |= (1 << i);
    }
    pkt->buttons_down = down;
    pkt->buttons_pressed = pressed;
    pkt->buttons_released = released;
}


void netgame_unpack_input(const InputPacket* pkt, int player_entity) {
    PlayerComponent* player = PlayerComponent_get(player_entity);
    if (!player) return;

    Controller* ctrl = &player->controller;

    ctrl->left_stick.x = pkt->left_stick_x;
    ctrl->left_stick.y = pkt->left_stick_y;
    ctrl->right_stick.x = pkt->right_stick_x;
    ctrl->right_stick.y = pkt->right_stick_y;
    ctrl->left_trigger = pkt->left_trigger;
    ctrl->right_trigger = pkt->right_trigger;

    for (int i = 0; i < 12; i++) {
        ctrl->buttons_down[i]    = (pkt->buttons_down    & (1 << i)) != 0;
        ctrl->buttons_pressed[i] = (pkt->buttons_pressed  & (1 << i)) != 0;
        ctrl->buttons_released[i] = (pkt->buttons_released & (1 << i)) != 0;
    }
}

#endif // __EMSCRIPTEN__
