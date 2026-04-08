#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "component.h"
#include "network.h"

#pragma pack(push, 1)

// Snapshot packet header: sent from host to clients every tick.
// Followed by variable-length entity data: for each entity,
// [entity_id(u16)] [binary_serialize_entity() output].
// After entity data: sound event count (u8) + sound events,
// then particle event count (u8) + particle events.
typedef struct {
    PacketHeader header;
    uint16_t entity_count;
} SnapshotPacket;

// Input packet sent from client to host
typedef struct {
    PacketHeader header;
    uint8_t player_slot;
    float left_stick_x;
    float left_stick_y;
    float right_stick_x;
    float right_stick_y;
    float left_trigger;
    float right_trigger;
    uint16_t buttons_down;      // 12 buttons packed as bits
    uint16_t buttons_pressed;
    uint16_t buttons_released;
} InputPacket;

#pragma pack(pop)


// Track which entities have been seen in snapshots (for implicit destroy).
// Indexed by LOCAL entity ID.
extern bool net_entity_seen[MAX_ENTITIES];

// Track the max entity id after map load (entities above this are runtime)
extern int net_map_max_entity;

// Entity ID mapping: host_id -> local_id for mismatched entities.
// -1 means identity mapping (host_id == local_id).
extern int net_host_to_local[MAX_ENTITIES];
extern int net_local_to_host[MAX_ENTITIES];

// Resolve a host entity ID to the local entity ID.
// Returns the local ID (may differ from host_id if there was a mismatch).
int net_resolve_id(int host_id);

// Clear entity ID mappings (call when starting a new game).
void net_clear_id_map(void);

// Host: build a snapshot of all dynamic entities into the send buffer.
// Returns the total packet size.
int netgame_build_snapshot(uint8_t* buf, int buf_size, uint32_t tick);

// Client: apply a received snapshot to the local game state.
void netgame_apply_snapshot(const uint8_t* buf, int size);

// Pack local player controller state into an InputPacket.
void netgame_pack_input(InputPacket* pkt, int player_entity, uint8_t player_slot, uint32_t tick);

// Host: unpack a received input packet into a player entity's controller.
void netgame_unpack_input(const InputPacket* pkt, int player_entity);

// Check if an entity is "dynamic" (needs to be synced).
// An entity is dynamic if it has dynamic components OR any ancestor in
// its coord->parent chain is dynamic.
bool netgame_is_dynamic(int entity);
