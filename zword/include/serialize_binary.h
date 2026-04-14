#pragma once

#include <stdbool.h>
#include <stdint.h>

// Binary component flags (which optional components an entity has).
// CoordinateComponent is always present and written unconditionally (no flag).
#define BFLAG_HAS_IMAGE       (1 << 0)
#define BFLAG_HAS_PHYSICS     (1 << 1)
#define BFLAG_HAS_COLLIDER    (1 << 2)
#define BFLAG_HAS_PLAYER      (1 << 3)
#define BFLAG_HAS_ENEMY       (1 << 4)
#define BFLAG_HAS_HEALTH      (1 << 5)
#define BFLAG_HAS_WEAPON      (1 << 6)
#define BFLAG_HAS_ITEM        (1 << 7)
#define BFLAG_HAS_WAYPOINT    (1 << 8)
#define BFLAG_HAS_PARTICLE    (1 << 9)
#define BFLAG_HAS_LIGHT       (1 << 10)
#define BFLAG_HAS_SOUND       (1 << 11)
#define BFLAG_HAS_AMMO        (1 << 12)
#define BFLAG_HAS_ANIMATION   (1 << 13)
#define BFLAG_HAS_DOOR        (1 << 14)
#define BFLAG_HAS_JOINT       (1 << 15)
#define BFLAG_HAS_VEHICLE     (1 << 16)
#define BFLAG_HAS_TEXT        (1 << 17)

// Serialize an entity's full state (Coordinate + all components) into a binary buffer.
// Wire format: [Coordinate fields (always)] [flags u32] [component data in flag order]
// Includes both creation-time parameters AND runtime state for all components.
// Returns number of bytes written, or 0 if buffer too small.
int binary_serialize_entity(uint8_t* buf, int buf_size, int entity);

// Deserialize an entity from binary data.
// Uses "get-or-add" pattern: for each component, checks if it already exists
// (via *_get()) and updates in place, or creates it (via *_add()) if missing.
// The entity must already exist (have a CoordinateComponent).
// When smooth is true, position and angle are blended toward the snapshot
// values instead of hard-snapped (used for client-side network smoothing).
// Returns number of bytes consumed, or 0 on error.
int binary_deserialize_entity(const uint8_t* buf, int buf_size, int entity, bool smooth);

// Skip past binary entity data without creating anything.
// Parses the data to determine its size.
// Returns number of bytes consumed, or 0 on error.
int binary_skip_entity(const uint8_t* buf, int buf_size);
