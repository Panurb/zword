#include <stdio.h>
#include <string.h>

#include "netgame.h"
#include "network.h"
#include "component.h"
#include "game.h"
#include "grid.h"
#include "image.h"
#include "serialize_binary.h"
#include "sound.h"
#include "particle.h"

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


// Check if an entity has dynamic components (not counting parent chain).
static bool has_dynamic_components(int entity) {
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


bool netgame_is_dynamic(int entity) {
    // Check own components
    if (has_dynamic_components(entity)) return true;

    // Walk parent chain: if any ancestor is dynamic, so are we
    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (coord && coord->parent >= 0) {
        return netgame_is_dynamic(coord->parent);
    }
    return false;
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

        // Write entity_id (u16) + binary_serialize_entity() output
        if (remaining < 2) {
            LOG_WARNING("Snapshot buffer full at %d entities", pkt->entity_count);
            break;
        }

        // Write entity_id
        uint16_t eid = (uint16_t)i;
        memcpy(data, &eid, 2);
        data += 2;
        remaining -= 2;

        // Write full entity state
        int written = binary_serialize_entity(data, remaining, i);
        if (written == 0) {
            // Not enough space, undo the entity_id write
            data -= 2;
            remaining += 2;
            LOG_WARNING("Snapshot buffer full at %d entities", pkt->entity_count);
            break;
        }

        data += written;
        remaining -= written;
        pkt->entity_count++;
    }

    // Append sound events by scanning pending one-shot SoundComponent events
    {
        uint8_t* count_ptr = data;
        data += 1;
        remaining -= 1;
        uint8_t sound_count = 0;

        for (int i = 0; i < game_data->components->entities; i++) {
            SoundComponent* scomp = SoundComponent_get(i);
            if (!scomp) continue;

            for (int j = 0; j < scomp->size; j++) {
                SoundEvent* event = scomp->events[j];
                if (!event) continue;
                if (event->channel != -1 || event->loop) continue;

                int idx = sound_index(event->filename);
                if (idx < 0) continue;
                if (remaining < 5) break;

                uint16_t eid = (uint16_t)i;
                uint16_t sidx = (uint16_t)idx;
                uint8_t vol = (uint8_t)(clamp(event->volume, 0.0f, 1.0f) * 255.0f);
                memcpy(data, &eid, 2);
                memcpy(data + 2, &sidx, 2);
                data[4] = vol;
                data += 5;
                remaining -= 5;
                sound_count++;
            }
        }

        *count_ptr = sound_count;
    }

    // Append particle events by scanning pending_burst on ParticleComponents
    // Wire format per event: [u16 entity_id][u16 count][float origin_x][float origin_y][float max_time]
    {
        uint8_t* count_ptr = data;
        data += 1;
        remaining -= 1;
        uint8_t particle_count = 0;

        for (int i = 0; i < game_data->components->entities; i++) {
            ParticleComponent* part = ParticleComponent_get(i);
            if (!part || part->pending_burst <= 0) continue;
            if (remaining < 16) break;

            uint16_t eid = (uint16_t)i;
            uint16_t count = (uint16_t)part->pending_burst;
            memcpy(data, &eid, 2);
            memcpy(data + 2, &count, 2);
            memcpy(data + 4, &part->origin.x, 4);
            memcpy(data + 8, &part->origin.y, 4);
            memcpy(data + 12, &part->max_time, 4);
            data += 16;
            remaining -= 16;
            part->pending_burst = 0;
            particle_count++;
        }

        *count_ptr = particle_count;
    }

    int total_size = (int)(data - buf);
    pkt->header.size = (uint16_t)total_size;

    return total_size;
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


// Remap a single entity ID field through net_resolve_id (if >= 0).
static int remap_id(int raw_host_id) {
    return (raw_host_id >= 0) ? net_resolve_id(raw_host_id) : raw_host_id;
}


// Post-pass: remap all cross-entity ID reference fields for one entity.
static void remap_entity_ids(int entity) {
    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (coord) {
        coord->parent = remap_id(coord->parent);
    }

    PlayerComponent* player = PlayerComponent_get(entity);
    if (player) {
        player->arms = remap_id(player->arms);
        player->vehicle = remap_id(player->vehicle);
        player->target = remap_id(player->target);
        player->grabbed_item = remap_id(player->grabbed_item);
        for (int j = 0; j < 4; j++) {
            player->inventory[j] = remap_id(player->inventory[j]);
        }
        for (int j = 0; j < 4; j++) {
            player->ammo[j] = remap_id(player->ammo[j]);
        }
        for (int j = 0; j < 3; j++) {
            player->keys[j] = remap_id(player->keys[j]);
        }
    }

    EnemyComponent* enemy = EnemyComponent_get(entity);
    if (enemy) {
        enemy->weapon = remap_id(enemy->weapon);
        enemy->target = remap_id(enemy->target);
    }

    ItemComponent* item = ItemComponent_get(entity);
    if (item) {
        for (int j = 0; j < 5; j++) {
            item->attachments[j] = remap_id(item->attachments[j]);
        }
    }

    VehicleComponent* vehicle = VehicleComponent_get(entity);
    if (vehicle) {
        for (int j = 0; j < 4; j++) {
            vehicle->riders[j] = remap_id(vehicle->riders[j]);
        }
    }

    JointComponent* joint = JointComponent_get(entity);
    if (joint) {
        joint->parent = remap_id(joint->parent);
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

    // =========================================================
    // Pass 1: Deserialize all entities
    // =========================================================
    for (int i = 0; i < pkt->entity_count; i++) {
        if (remaining < 2) {
            LOG_WARNING("Snapshot truncated at entity %d/%d", i, pkt->entity_count);
            break;
        }

        // Read host entity_id
        uint16_t host_eid;
        memcpy(&host_eid, data, 2);
        data += 2;
        remaining -= 2;
        int host_id = (int)host_eid;

        // Resolve to local entity ID
        int entity = net_resolve_id(host_id);

        // Check if entity exists locally
        CoordinateComponent* coord = CoordinateComponent_get(entity);
        if (!coord) {
            // Entity doesn't exist — create it
            int created = create_entity();
            if (created == -1) {
                // Can't create; skip this entity's binary data
                int skipped = binary_skip_entity(data, remaining);
                if (skipped > 0) {
                    data += skipped;
                    remaining -= skipped;
                }
                LOG_WARNING("Failed to create entity for host_id=%d", host_id);
                continue;
            }

            Vector2f zero_pos = { 0.0f, 0.0f };
            CoordinateComponent_add(created, zero_pos, 0.0f);

            // Record ID mapping if local ID differs from host ID
            if (created != host_id) {
                if (host_id >= 0 && host_id < MAX_ENTITIES &&
                    created >= 0 && created < MAX_ENTITIES) {
                    net_host_to_local[host_id] = created;
                    net_local_to_host[created] = host_id;
                }
            }

            entity = created;
        }

        // Deserialize all component data (updates in place if exists, creates if not)
        int consumed = binary_deserialize_entity(data, remaining, entity);
        if (consumed == 0) {
            LOG_WARNING("Failed to deserialize entity snapshot for host_id=%d", host_id);
            // Try to skip
            int skipped = binary_skip_entity(data, remaining);
            if (skipped > 0) {
                data += skipped;
                remaining -= skipped;
            }
            continue;
        }

        data += consumed;
        remaining -= consumed;

        // Mark as seen and in this snapshot
        if (entity >= 0 && entity < MAX_ENTITIES) {
            net_entity_seen[entity] = true;
            in_snapshot[entity] = true;
        }
    }

    // =========================================================
    // Pass 2: Remap all cross-entity ID reference fields
    // =========================================================
    for (int i = 0; i < game_data->components->entities; i++) {
        if (!in_snapshot[i]) continue;
        remap_entity_ids(i);
    }

    // =========================================================
    // Pass 3: Rebuild parent-child relationships
    // =========================================================

    // Clear children lists of all snapshot entities
    for (int i = 0; i < game_data->components->entities; i++) {
        if (!in_snapshot[i]) continue;
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;
        List_clear(coord->children);
    }

    // Re-establish parent-child links for ALL entities (not just snapshot ones).
    // This handles both snapshot entities and static entities that might be
    // children of snapshot entities.
    for (int i = 0; i < game_data->components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;
        if (coord->parent < 0) continue;

        CoordinateComponent* parent_coord = CoordinateComponent_get(coord->parent);
        if (!parent_coord) continue;

        // Only rebuild if parent or child is a snapshot entity
        if (!in_snapshot[i] && !in_snapshot[coord->parent]) continue;

        // Add to parent's children list if not already present
        if (!List_find(parent_coord->children, i)) {
            List_append(parent_coord->children, i);
        }
    }

    // =========================================================
    // Pass 4: Implicit destroy — entities seen before but missing from snapshot
    // =========================================================
    for (int i = 0; i < game_data->components->entities; i++) {
        if (net_entity_seen[i] && !in_snapshot[i]) {
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
                    mark_children_unseen(i);
                    destroy_entity_recursive(i);
                }
            }
            net_entity_seen[i] = false;

            // Clean up ID mapping
            int host_id = net_local_to_host[i];
            if (host_id >= 0 && host_id < MAX_ENTITIES) {
                net_host_to_local[host_id] = -1;
                net_local_to_host[i] = -1;
            }
        }
    }

    // =========================================================
    // Pass 5: Replay sound and particle events from the snapshot
    // =========================================================

    // Sound events
    if (remaining >= 1) {
        uint8_t sound_count = *data;
        data += 1;
        remaining -= 1;

        for (int i = 0; i < sound_count; i++) {
            if (remaining < 5) break;

            uint16_t host_eid;
            uint16_t sidx;
            uint8_t vol;
            memcpy(&host_eid, data, 2);
            memcpy(&sidx, data + 2, 2);
            vol = data[4];
            data += 5;
            remaining -= 5;

            int local_id = net_resolve_id((int)host_eid);
            if (local_id >= 0 && local_id < MAX_ENTITIES && sidx < resources.sounds_size) {
                SoundComponent* sc = SoundComponent_get(local_id);
                if (sc) {
                    float volume = (float)vol / 255.0f;
                    add_sound(local_id, resources.sound_names[sidx], volume, 1.0f);
                }
            }
        }
    }

    // Particle events
    if (remaining >= 1) {
        uint8_t particle_count = *data;
        data += 1;
        remaining -= 1;

        for (int i = 0; i < particle_count; i++) {
            if (remaining < 16) break;

            uint16_t host_eid;
            uint16_t count;
            float origin_x, origin_y, max_time;
            memcpy(&host_eid, data, 2);
            memcpy(&count, data + 2, 2);
            memcpy(&origin_x, data + 4, 4);
            memcpy(&origin_y, data + 8, 4);
            memcpy(&max_time, data + 12, 4);
            data += 16;
            remaining -= 16;

            int local_id = net_resolve_id((int)host_eid);
            if (local_id >= 0 && local_id < MAX_ENTITIES) {
                ParticleComponent* part = ParticleComponent_get(local_id);
                if (part) {
                    part->origin.x = origin_x;
                    part->origin.y = origin_y;
                    part->max_time = max_time;
                    add_particles(local_id, (int)count);
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
        // OR edge-triggered flags so that if multiple packets arrive in the
        // same host tick, earlier press/release events are not overwritten.
        // These flags are cleared after the state machine runs each tick.
        ctrl->buttons_pressed[i]  = ctrl->buttons_pressed[i]  || ((pkt->buttons_pressed  & (1 << i)) != 0);
        ctrl->buttons_released[i] = ctrl->buttons_released[i] || ((pkt->buttons_released & (1 << i)) != 0);
    }
}
