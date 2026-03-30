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

// Component flags for variable-length entity snapshots
#define SNAP_HAS_PHYSICS  (1 << 0)
#define SNAP_HAS_HEALTH   (1 << 1)
#define SNAP_HAS_PLAYER   (1 << 2)
#define SNAP_HAS_ENEMY    (1 << 3)
#define SNAP_HAS_WEAPON   (1 << 4)
#define SNAP_HAS_DOOR     (1 << 5)
#define SNAP_HAS_ITEM     (1 << 6)
#define SNAP_HAS_AMMO     (1 << 7)
#define SNAP_HAS_LIGHT    (1 << 8)
#define SNAP_HAS_IMAGE    (1 << 9)
#define SNAP_HAS_ANIM     (1 << 10)
#define SNAP_HAS_VEHICLE  (1 << 11)

// Per-entity snapshot: variable-length, base header + optional component sections.
// Written as raw bytes into the snapshot buffer (not an array of fixed-size structs).
#pragma pack(push, 1)

// Always present for every synced entity (21 bytes)
typedef struct {
    uint16_t entity_id;
    uint8_t net_type;       // NetEntityType
    uint16_t comp_flags;    // Bitmask of SNAP_HAS_* indicating which sections follow
    float pos_x;
    float pos_y;
    float angle;
    int16_t parent;         // coord->parent (-1 if none)
    float scale_x;
    float scale_y;
} SnapBase;

// SNAP_HAS_PHYSICS (8 bytes)
typedef struct {
    float vel_x;
    float vel_y;
} SnapPhysics;

// SNAP_HAS_HEALTH (7 bytes)
typedef struct {
    int16_t health;
    int16_t max_health;
    uint8_t dead;           // bool
    uint8_t status_type;    // StatusEffect enum
    uint8_t _pad;           // padding for alignment
} SnapHealth;

// SNAP_HAS_PLAYER (46 bytes)
typedef struct {
    uint8_t state;          // PlayerState
    int32_t money;
    uint8_t item;           // current inventory slot index
    int16_t inventory[4];   // entity IDs of held items (-1 = empty)
    int16_t ammo[4];        // entity IDs of ammo slots
    uint8_t keys[3];        // key flags
    int16_t arms;           // arms child entity
    int16_t vehicle;        // vehicle entity (-1 if none)
    int16_t target;         // target entity (-1 if none)
    float use_timer;
    int16_t grabbed_item;   // grabbed item entity (-1 if none)
    uint8_t won;            // bool
    float money_timer;
    int32_t money_increment;
} SnapPlayer;

// SNAP_HAS_ENEMY (10 bytes)
typedef struct {
    uint8_t state;          // EnemyState
    int16_t weapon;         // weapon child entity
    uint8_t boss;           // bool
    int32_t bounty;
    int16_t target;         // target entity
} SnapEnemy;

// SNAP_HAS_WEAPON (13 bytes)
typedef struct {
    float recoil;
    float spread;
    float cooldown;
    int16_t magazine;       // current ammo in magazine
    uint8_t reloading;      // bool
    uint8_t ammo_type;      // AmmoType enum
} SnapWeapon;              // actually 14 bytes

// SNAP_HAS_DOOR (6 bytes)
typedef struct {
    uint8_t locked;         // bool
    int16_t price;
    uint8_t key;            // DoorKey enum
    uint8_t _pad[2];        // padding
} SnapDoor;

// SNAP_HAS_ITEM (14 bytes)
typedef struct {
    int16_t price;
    int16_t attachments[5]; // child entity IDs (-1 = empty)
    uint8_t size;           // number of attachment slots
    uint8_t type;           // ItemType enum
} SnapItem;

// SNAP_HAS_AMMO (3 bytes)
typedef struct {
    uint8_t type;           // AmmoType enum
    int16_t size;           // ammo count
} SnapAmmo;

// SNAP_HAS_LIGHT (5 bytes)
typedef struct {
    uint8_t enabled;        // bool
    float brightness;
} SnapLight;

// SNAP_HAS_IMAGE (5 bytes)
typedef struct {
    float alpha;
    uint8_t layer;          // Layer enum
} SnapImage;

// SNAP_HAS_ANIM (1 byte)
typedef struct {
    uint8_t current_frame;
} SnapAnim;

// SNAP_HAS_VEHICLE (15 bytes)
typedef struct {
    float fuel;
    uint8_t on_road;        // bool
    int16_t riders[4];      // entity IDs of riders (-1 = empty)
    uint8_t size;           // number of seats
} SnapVehicle;

typedef struct {
    PacketHeader header;
    uint16_t entity_count;
    // Followed by variable-length entity data (SnapBase + optional sections)
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

// Check if an entity is "dynamic" (needs to be synced)
bool netgame_is_dynamic(int entity);

// Client: create a runtime entity from a SnapBase (uses net_type and position)
int netgame_create_entity_from_snapshot(const SnapBase* base);

#endif // __EMSCRIPTEN__
