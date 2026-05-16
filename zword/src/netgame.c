#include <stdio.h>
#include <string.h>

#include "app.h"
#include "netgame.h"
#include "network.h"
#include "component.h"
#include "game.h"
#include "grid.h"
#include "image.h"
#include "input.h"
#include "serialize_binary.h"
#include "sound.h"
#include "particle.h"

bool net_entity_seen[MAX_ENTITIES];


// Check if an entity has dynamic components (not counting parent chain).
static bool has_dynamic_components(int entity) {
    if (CoordinateComponent_get(entity)->lifetime > 0.0f) return true;
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
    pkt->wave = 0;
    pkt->wave_delay = 0.0f;

    if (game_data->game_mode == MODE_SURVIVAL) {
        pkt->wave = (uint16_t)game_data->wave;
        pkt->wave_delay = game_data->wave_delay;
    }

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
        mark_children_unseen(child);
    }
}


void netgame_apply_snapshot(const uint8_t* buf, int size) {
    if (size < (int)sizeof(SnapshotPacket)) return;

    const SnapshotPacket* pkt = (const SnapshotPacket*)buf;
    if (pkt->header.type != PACKET_SNAPSHOT) return;

    if (game_data->game_mode == MODE_SURVIVAL) {
        game_data->wave = pkt->wave;
        game_data->wave_delay = pkt->wave_delay;
    }

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

        // Resolve to local entity ID
        Entity entity = (int)host_eid;

        // Check if entity exists locally
        CoordinateComponent* coord = CoordinateComponent_get(entity);
        bool is_new = !coord;
        if (!coord) {
            // Entity doesn't exist — create it
            create_entity_with_id(entity);
            CoordinateComponent_add(entity, zeros(), 0.0f);
        }

        // Deserialize all component data (updates in place if exists, creates if not)
        int consumed = binary_deserialize_entity(data, remaining, entity, !is_new);
        if (consumed == 0) {
            LOG_WARNING("Failed to deserialize entity snapshot for entity=%d", entity);
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
    // Pass 2: Rebuild parent-child relationships
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
    // Pass 3: Implicit destroy — entities seen before but missing from snapshot
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
        }
    }

    // =========================================================
    // Pass 4: Replay sound and particle events from the snapshot
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

            Entity entity = (int)host_eid;
            SoundComponent* sc = SoundComponent_get(entity);
            if (sc) {
                float volume = (float)vol / 255.0f;
                add_sound(entity, resources.sound_names[sidx], volume, 1.0f);
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

            Entity entity = (int)host_eid;
            ParticleComponent* part = ParticleComponent_get(entity);
            if (part) {
                part->origin.x = origin_x;
                part->origin.y = origin_y;
                part->max_time = max_time;
                add_particles(entity, (int)count);
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


bool netgame_client_send_input(bool neutral) {
    int slot_idx = 0;
    ListNode* pnode;
    FOREACH(pnode, game_data->components->player.order) {
        if (slot_idx == network.local_player_slot) {
            if (neutral) {
                PlayerComponent* player = PlayerComponent_get(pnode->value);
                memset(&player->controller, 0, sizeof(player->controller));
                player->controller.joystick = app.player_controllers[network.local_player_slot];
            } else {
                update_controller(game_data->camera, pnode->value);
            }

            InputPacket input_pkt;
            netgame_pack_input(&input_pkt, pnode->value, (uint8_t)network.local_player_slot, network.tick);
            network_send_to_host(&input_pkt, sizeof(input_pkt));
            return true;
        }
        slot_idx++;
    }
    return false;
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
