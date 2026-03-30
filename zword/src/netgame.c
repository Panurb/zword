#include <stdio.h>
#include <string.h>

#include "netgame.h"
#include "network.h"
#include "component.h"
#include "game.h"
#include "grid.h"
#include "image.h"
#include "serialize_binary.h"

#ifndef __EMSCRIPTEN__

bool net_entity_seen[MAX_ENTITIES];
int net_map_max_entity = 0;
int net_host_to_local[MAX_ENTITIES];
int net_local_to_host[MAX_ENTITIES];
bool net_entity_needs_full[MAX_ENTITIES];
bool net_entity_sent[MAX_ENTITIES];


int net_resolve_id(int host_id) {
    if (host_id < 0 || host_id >= MAX_ENTITIES) return host_id;
    return (net_host_to_local[host_id] >= 0) ? net_host_to_local[host_id] : host_id;
}


void net_clear_id_map(void) {
    memset(net_host_to_local, -1, sizeof(net_host_to_local));
    memset(net_local_to_host, -1, sizeof(net_local_to_host));
    memset(net_entity_needs_full, 0, sizeof(net_entity_needs_full));
    memset(net_entity_sent, 0, sizeof(net_entity_sent));
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

    // Check if this entity needs a full creation snapshot
    bool is_full = net_entity_needs_full[entity];
    if (is_full) {
        flags |= SNAP_IS_FULL;
    }

    // Calculate size for the delta portion (SnapBase + optional component sections)
    int delta_size = (int)sizeof(SnapBase);
    if (flags & SNAP_HAS_PHYSICS) delta_size += (int)sizeof(SnapPhysics);
    if (flags & SNAP_HAS_HEALTH)  delta_size += (int)sizeof(SnapHealth);
    if (flags & SNAP_HAS_PLAYER)  delta_size += (int)sizeof(SnapPlayer);
    if (flags & SNAP_HAS_ENEMY)   delta_size += (int)sizeof(SnapEnemy);
    if (flags & SNAP_HAS_WEAPON)  delta_size += (int)sizeof(SnapWeapon);
    if (flags & SNAP_HAS_DOOR)    delta_size += (int)sizeof(SnapDoor);
    if (flags & SNAP_HAS_ITEM)    delta_size += (int)sizeof(SnapItem);
    if (flags & SNAP_HAS_AMMO)    delta_size += (int)sizeof(SnapAmmo);
    if (flags & SNAP_HAS_LIGHT)   delta_size += (int)sizeof(SnapLight);
    if (flags & SNAP_HAS_IMAGE)   delta_size += (int)sizeof(SnapImage);
    if (flags & SNAP_HAS_ANIM)    delta_size += (int)sizeof(SnapAnim);
    if (flags & SNAP_HAS_VEHICLE) delta_size += (int)sizeof(SnapVehicle);

    if (delta_size > remaining) return 0;

    uint8_t* ptr = buf;

    // Write base
    {
        SnapBase base;
        base.entity_id = (uint16_t)entity;
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

    // Write optional delta sections in flag order
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

    // If SNAP_IS_FULL, append the binary creation data after the delta sections
    if (is_full) {
        int full_remaining = remaining - (int)(ptr - buf);
        int full_written = binary_serialize_entity(ptr, full_remaining, entity);
        if (full_written == 0) {
            // Not enough space for full snapshot -- skip it, will retry next frame
            return 0;
        }
        ptr += full_written;
        net_entity_needs_full[entity] = false;
        net_entity_sent[entity] = true;
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

        // Detect newly created entities that haven't been sent yet
        if (!net_entity_sent[i]) {
            net_entity_needs_full[i] = true;
        }

        int written = build_entity_snapshot(data, remaining, i);
        if (written == 0) {
            LOG_WARNING("Snapshot buffer full at %d entities", pkt->entity_count);
            break;
        }

        data += written;
        remaining -= written;
        pkt->entity_count++;

        // Mark as sent (even if it was already sent; harmless for non-full)
        net_entity_sent[i] = true;
    }

    int total_size = (int)(data - buf);
    pkt->header.size = (uint16_t)total_size;

    return total_size;
}


// Read a single variable-length entity snapshot from buf and apply it.
// Returns number of bytes consumed, or 0 on error.
static int apply_entity_snapshot(const uint8_t* buf, int remaining) {
    if (remaining < (int)sizeof(SnapBase)) return 0;

    const SnapBase* base = (const SnapBase*)buf;
    uint16_t flags = base->comp_flags;
    int host_id = (int)base->entity_id;
    bool is_full = (flags & SNAP_IS_FULL) != 0;

    // Calculate expected size for the delta portion
    int delta_size = (int)sizeof(SnapBase);
    if (flags & SNAP_HAS_PHYSICS) delta_size += (int)sizeof(SnapPhysics);
    if (flags & SNAP_HAS_HEALTH)  delta_size += (int)sizeof(SnapHealth);
    if (flags & SNAP_HAS_PLAYER)  delta_size += (int)sizeof(SnapPlayer);
    if (flags & SNAP_HAS_ENEMY)   delta_size += (int)sizeof(SnapEnemy);
    if (flags & SNAP_HAS_WEAPON)  delta_size += (int)sizeof(SnapWeapon);
    if (flags & SNAP_HAS_DOOR)    delta_size += (int)sizeof(SnapDoor);
    if (flags & SNAP_HAS_ITEM)    delta_size += (int)sizeof(SnapItem);
    if (flags & SNAP_HAS_AMMO)    delta_size += (int)sizeof(SnapAmmo);
    if (flags & SNAP_HAS_LIGHT)   delta_size += (int)sizeof(SnapLight);
    if (flags & SNAP_HAS_IMAGE)   delta_size += (int)sizeof(SnapImage);
    if (flags & SNAP_HAS_ANIM)    delta_size += (int)sizeof(SnapAnim);
    if (flags & SNAP_HAS_VEHICLE) delta_size += (int)sizeof(SnapVehicle);

    if (delta_size > remaining) return 0;

    // Resolve host entity ID to local entity ID
    int entity = net_resolve_id(host_id);

    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (!coord) {
        // Entity doesn't exist locally. We need a full snapshot to create it.
        if (!is_full) {
            // No creation data. This is a map entity that was already destroyed
            // locally, or a runtime entity whose full snapshot hasn't arrived yet.
            // Skip it.
            return delta_size;
        }

        // Create the entity from binary creation data.
        // The binary data follows after the delta sections.
        const uint8_t* full_data = buf + delta_size;
        int full_remaining = remaining - delta_size;

        // First, create a bare entity with a CoordinateComponent
        int created = create_entity();
        if (created == -1) return delta_size;

        Vector2f pos = { base->pos_x, base->pos_y };
        CoordinateComponent_add(created, pos, base->angle);

        // Deserialize all components from the binary data
        int full_consumed = binary_deserialize_entity(full_data, full_remaining, created);
        if (full_consumed == 0) {
            // Failed to deserialize -- destroy the partially created entity
            destroy_entity(created);
            LOG_WARNING("Failed to deserialize full entity snapshot for host_id=%d", host_id);
            return delta_size;
        }

        // Record ID mapping if local ID differs from host ID
        if (created != host_id) {
            LOG_WARNING("Entity ID mismatch: host=%d local=%d", host_id, created);
            if (host_id >= 0 && host_id < MAX_ENTITIES &&
                created >= 0 && created < MAX_ENTITIES) {
                net_host_to_local[host_id] = created;
                net_local_to_host[created] = host_id;
            }
        }

        coord = CoordinateComponent_get(created);
        if (!coord) return delta_size + full_consumed;
        entity = created;

        // Mark as seen
        net_entity_seen[entity] = true;

        // Apply base fields on top of the created entity
        coord->position.x = base->pos_x;
        coord->position.y = base->pos_y;
        coord->angle = base->angle;
        coord->parent = (base->parent >= 0) ? net_resolve_id((int)base->parent) : (int)base->parent;
        coord->scale.x = base->scale_x;
        coord->scale.y = base->scale_y;

        // Now apply the delta state sections (overwrite runtime state)
        goto apply_delta_sections;

        // Return total size = delta + full
        // (We'll reach this via the normal flow after apply_delta_sections)
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

apply_delta_sections:
    ;  // empty statement after label required by C

    // Walk through optional delta sections
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

    // If SNAP_IS_FULL was set, skip past the binary creation data too.
    // For existing entities we already consumed it during creation above.
    // For the case where entity already existed, we just need to skip it.
    if (is_full) {
        const uint8_t* full_data = ptr;
        int full_remaining = remaining - (int)(ptr - buf);
        int full_consumed = binary_skip_entity(full_data, full_remaining);
        if (full_consumed > 0) {
            return delta_size + full_consumed;
        }
        // If parsing failed, just return delta_size (best effort)
        return delta_size;
    }

    return delta_size;
}


// Helper: recursively mark children as not-seen before destroy
static void mark_children_unseen(int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (!coord) return;
    for (ListNode* node = coord->children->head; node; node = node->next) {
        int child = node->value;
        net_entity_seen[child] = false;
        // Clean up ID mapping for child
        int host_id = net_local_to_host[child];
        if (host_id >= 0 && host_id < MAX_ENTITIES) {
            net_host_to_local[host_id] = -1;
            net_local_to_host[child] = -1;
        }
        mark_children_unseen(child);
    }
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
                    // Mark all children as unseen before destroying recursively
                    mark_children_unseen(i);
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

    // Rebuild parent-child relationships for all entities in the snapshot.
    // This ensures the parent's children list is always consistent with
    // the child's coord->parent field (which was set during apply_entity_snapshot).
    for (int i = 0; i < game_data->components->entities; i++) {
        if (!in_snapshot[i]) continue;
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;

        // Clear this entity's children list -- we'll rebuild it below
        // (only for snapshot entities; static entities keep their lists)
        List_clear(coord->children);
    }

    // Second pass: re-establish parent-child links
    for (int i = 0; i < game_data->components->entities; i++) {
        if (!in_snapshot[i]) continue;
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;

        if (coord->parent != -1) {
            CoordinateComponent* parent_coord = CoordinateComponent_get(coord->parent);
            if (parent_coord) {
                // Only add to children list if not already present
                if (!List_find(parent_coord->children, i)) {
                    List_append(parent_coord->children, i);
                }
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
