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


static void build_entity_snapshot(EntitySnapshot* snap, int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(entity);
    PhysicsComponent* phys = PhysicsComponent_get(entity);
    HealthComponent* health = HealthComponent_get(entity);
    ImageComponent* image = ImageComponent_get(entity);
    PlayerComponent* player = PlayerComponent_get(entity);
    EnemyComponent* enemy = EnemyComponent_get(entity);
    AnimationComponent* anim = AnimationComponent_get(entity);

    snap->entity_id = (uint16_t)entity;
    snap->net_type = coord->net_type;

    // Use world position for root entities, local for children
    if (coord->parent == -1) {
        snap->pos_x = coord->position.x;
        snap->pos_y = coord->position.y;
    } else {
        snap->pos_x = coord->position.x;
        snap->pos_y = coord->position.y;
    }
    snap->angle = coord->angle;

    if (phys) {
        snap->vel_x = phys->velocity.x;
        snap->vel_y = phys->velocity.y;
    } else {
        snap->vel_x = 0.0f;
        snap->vel_y = 0.0f;
    }

    if (health) {
        snap->health = (int16_t)health->health;
        snap->max_health = (int16_t)health->max_health;
        snap->flags = health->dead ? 1 : 0;
    } else {
        snap->health = 0;
        snap->max_health = 0;
        snap->flags = 0;
    }

    if (player) {
        snap->state = (uint8_t)player->state;
    } else if (enemy) {
        snap->state = (uint8_t)enemy->state;
    } else {
        snap->state = 0;
    }

    if (anim) {
        snap->anim_frame = (uint8_t)anim->current_frame;
    } else {
        snap->anim_frame = 0;
    }

    if (image) {
        snap->alpha = image->alpha;
    } else {
        snap->alpha = 1.0f;
    }

    snap->scale_x = coord->scale.x;
    snap->scale_y = coord->scale.y;

    // Debug: log player entity snapshot building
    if (player && (network.tick % 60 == 0)) {
        LOG_INFO("[HOST] build_snapshot: player entity=%d pos=(%.1f,%.1f) vel=(%.2f,%.2f) state=%d",
            entity, snap->pos_x, snap->pos_y, snap->vel_x, snap->vel_y, snap->state);
    }
}


int netgame_build_snapshot(uint8_t* buf, int buf_size, uint32_t tick) {
    SnapshotPacket* pkt = (SnapshotPacket*)buf;
    pkt->header.type = PACKET_SNAPSHOT;
    pkt->header.tick = tick;
    pkt->entity_count = 0;

    EntitySnapshot* snaps = (EntitySnapshot*)(buf + sizeof(SnapshotPacket));
    int max_entities = (buf_size - (int)sizeof(SnapshotPacket)) / (int)sizeof(EntitySnapshot);

    for (int i = 0; i < game_data->components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;

        // Skip cameras, widgets, menu entities
        if (CameraComponent_get(i)) continue;
        if (WidgetComponent_get(i)) continue;

        if (!netgame_is_dynamic(i)) continue;

        if (pkt->entity_count >= (uint16_t)max_entities) {
            LOG_WARNING("Snapshot buffer full at %d entities", pkt->entity_count);
            break;
        }

        build_entity_snapshot(&snaps[pkt->entity_count], i);
        pkt->entity_count++;
    }

    int total_size = (int)sizeof(SnapshotPacket) + pkt->entity_count * (int)sizeof(EntitySnapshot);
    pkt->header.size = (uint16_t)total_size;

    return total_size;
}


int netgame_create_entity_from_snapshot(const EntitySnapshot* snap) {
    // Runtime-created entity: use factory function based on net_type,
    // then overwrite state from snapshot.
    // For map entities we should never get here (they already exist).
    Vector2f pos = { snap->pos_x, snap->pos_y };
    int entity = -1;

    switch ((NetEntityType)snap->net_type) {
        case NET_ENTITY_ZOMBIE:
            entity = create_zombie(pos, snap->angle);
            break;
        case NET_ENTITY_FARMER:
            entity = create_farmer(pos, snap->angle);
            break;
        case NET_ENTITY_PRIEST:
            entity = create_priest(pos, snap->angle);
            break;
        case NET_ENTITY_BIG_BOY:
            entity = create_big_boy(pos, snap->angle);
            break;
        case NET_ENTITY_BOSS:
            entity = create_boss(pos, snap->angle);
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
            create_energy(pos, (Vector2f){ snap->vel_x, snap->vel_y });
            // create_energy doesn't return entity id, find it
            // It was the last created entity
            for (int i = game_data->components->entities - 1; i >= 0; i--) {
                CoordinateComponent* c = CoordinateComponent_get(i);
                if (c) { entity = i; break; }
            }
            break;
        case NET_ENTITY_FLAME:
            entity = create_flame(pos, (Vector2f){ snap->vel_x, snap->vel_y }, -1);
            break;
        case NET_ENTITY_SPLASH:
            entity = create_splash(pos, (Vector2f){ snap->vel_x, snap->vel_y });
            break;
        case NET_ENTITY_FREEZE:
            entity = create_freeze(pos, (Vector2f){ snap->vel_x, snap->vel_y });
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


static void apply_entity_snapshot(const EntitySnapshot* snap) {
    int entity = (int)snap->entity_id;

    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (!coord) {
        // Entity doesn't exist locally. 
        // If it's a map entity (net_type == NONE), it was destroyed on host. Ignore.
        if (snap->net_type == NET_ENTITY_NONE) return;

        // Runtime entity: create it using the factory
        int created = netgame_create_entity_from_snapshot(snap);
        if (created == -1) return;

        // The factory might have created it at a different ID than what the host has.
        // For now, we assume entity IDs align (same create_entity() allocation order).
        // If created != entity, we have an ID mismatch problem.
        // TODO: handle ID mismatch with an entity ID mapping table
        if (created != entity) {
            LOG_WARNING("Entity ID mismatch: host=%d local=%d (net_type=%d)", 
                        entity, created, snap->net_type);
        }

        coord = CoordinateComponent_get(created);
        if (!coord) return;
        entity = created;
    }

    // Mark as seen
    net_entity_seen[entity] = true;

    // Update position (local coordinates, same as host stores)
    coord->position.x = snap->pos_x;
    coord->position.y = snap->pos_y;
    coord->angle = snap->angle;
    coord->scale.x = snap->scale_x;
    coord->scale.y = snap->scale_y;

    // Update physics velocity
    PhysicsComponent* phys = PhysicsComponent_get(entity);
    if (phys) {
        phys->velocity.x = snap->vel_x;
        phys->velocity.y = snap->vel_y;
    }

    // Update health
    HealthComponent* health = HealthComponent_get(entity);
    if (health) {
        health->health = (int)snap->health;
        health->max_health = (int)snap->max_health;
        health->dead = (snap->flags & 1) != 0;
    }

    // Update state
    PlayerComponent* player = PlayerComponent_get(entity);
    EnemyComponent* enemy = EnemyComponent_get(entity);
    if (player) {
        player->state = (PlayerState)snap->state;

        // Debug: log player entity snapshot application
        if (network.mode == NET_MODE_CLIENT && (network.tick % 60 == 0)) {
            LOG_INFO("[CLIENT] apply_snapshot: player entity=%d pos=(%.1f,%.1f) vel=(%.2f,%.2f) state=%d",
                entity, snap->pos_x, snap->pos_y, snap->vel_x, snap->vel_y, snap->state);
        }
    } else if (enemy) {
        enemy->state = (EnemyState)snap->state;
    }

    // Update animation
    AnimationComponent* anim = AnimationComponent_get(entity);
    if (anim) {
        anim->current_frame = (int)snap->anim_frame;
    }

    // Update image alpha
    ImageComponent* image = ImageComponent_get(entity);
    if (image) {
        image->alpha = snap->alpha;
    }
}


void netgame_apply_snapshot(const uint8_t* buf, int size) {
    if (size < (int)sizeof(SnapshotPacket)) return;

    const SnapshotPacket* pkt = (const SnapshotPacket*)buf;
    if (pkt->header.type != PACKET_SNAPSHOT) return;

    int expected_size = (int)sizeof(SnapshotPacket) + pkt->entity_count * (int)sizeof(EntitySnapshot);
    if (size < expected_size) {
        LOG_WARNING("Snapshot packet too small: %d < %d", size, expected_size);
        return;
    }

    const EntitySnapshot* snaps = (const EntitySnapshot*)(buf + sizeof(SnapshotPacket));

    // Track which entities are in this snapshot
    bool in_snapshot[MAX_ENTITIES];
    memset(in_snapshot, 0, sizeof(in_snapshot));

    // Apply each entity snapshot
    for (int i = 0; i < pkt->entity_count; i++) {
        int eid = (int)snaps[i].entity_id;
        if (eid >= 0 && eid < MAX_ENTITIES) {
            in_snapshot[eid] = true;
        }
        apply_entity_snapshot(&snaps[i]);
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
