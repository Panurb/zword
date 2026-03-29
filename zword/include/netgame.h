#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "component.h"

// Entity types for runtime-created entities that need to be synced.
// Map-loaded entities don't need this since clients load the same map.
// Available on all platforms so factory functions can set net_type unconditionally.
typedef enum {
    NET_ENTITY_NONE = 0,
    // Enemies
    NET_ENTITY_ZOMBIE,
    NET_ENTITY_FARMER,
    NET_ENTITY_PRIEST,
    NET_ENTITY_BIG_BOY,
    NET_ENTITY_BOSS,
    // Projectiles
    NET_ENTITY_ENERGY,
    NET_ENTITY_FLAME,
    NET_ENTITY_SPLASH,
    NET_ENTITY_FREEZE,
    NET_ENTITY_ROPE,
    // Items
    NET_ENTITY_BANDAGE,
    NET_ENTITY_AMMO_PISTOL,
    NET_ENTITY_AMMO_RIFLE,
    NET_ENTITY_AMMO_SHOTGUN,
    // Weapons
    NET_ENTITY_PISTOL,
    NET_ENTITY_SHOTGUN,
    NET_ENTITY_SAWED_OFF,
    NET_ENTITY_RIFLE,
    NET_ENTITY_ASSAULT_RIFLE,
    NET_ENTITY_SMG,
    NET_ENTITY_AXE,
    NET_ENTITY_SWORD,
    // Decals
    NET_ENTITY_DECAL,
} NetEntityType;

#ifndef __EMSCRIPTEN__

#include "network.h"

// Per-entity snapshot data sent from host to clients
#pragma pack(push, 1)

typedef struct {
    uint16_t entity_id;
    uint8_t net_type;       // NetEntityType
    float pos_x;
    float pos_y;
    float angle;
    float vel_x;
    float vel_y;
    int16_t health;
    int16_t max_health;
    uint8_t state;          // PlayerState or EnemyState
    uint8_t anim_frame;
    float alpha;
    float scale_x;
    float scale_y;
    uint8_t flags;          // bit 0: dead
} EntitySnapshot;

typedef struct {
    PacketHeader header;
    uint16_t entity_count;
    // Followed by entity_count * EntitySnapshot
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


// Track which entities have been seen in snapshots (for implicit destroy)
extern bool net_entity_seen[MAX_ENTITIES];

// Track the max entity id after map load (entities above this are runtime)
extern int net_map_max_entity;

// Host: build a snapshot of all dynamic entities into the send buffer.
// Returns the total packet size.
int netgame_build_snapshot(uint8_t* buf, int buf_size, uint32_t tick);

// Client: apply a received snapshot to the local game state.
void netgame_apply_snapshot(const uint8_t* buf, int size);

// Pack local player controller state into an InputPacket.
void netgame_pack_input(InputPacket* pkt, int player_entity, uint8_t player_slot, uint32_t tick);

// Host: unpack a received input packet into a player entity's controller.
void netgame_unpack_input(const InputPacket* pkt, int player_entity);

// Check if an entity is "dynamic" (needs to be synced)
bool netgame_is_dynamic(int entity);

// Client: create a runtime entity from a snapshot entry
int netgame_create_entity_from_snapshot(const EntitySnapshot* snap);

#endif // __EMSCRIPTEN__
