#pragma once

#include <stdint.h>

// Binary component creation flags (which components an entity has).
// Used in the SNAP_IS_FULL binary creation payload.
#define BCREATE_HAS_IMAGE       (1 << 0)
#define BCREATE_HAS_PHYSICS     (1 << 1)
#define BCREATE_HAS_COLLIDER    (1 << 2)
#define BCREATE_HAS_PLAYER      (1 << 3)
#define BCREATE_HAS_ENEMY       (1 << 4)
#define BCREATE_HAS_HEALTH      (1 << 5)
#define BCREATE_HAS_WEAPON      (1 << 6)
#define BCREATE_HAS_ITEM        (1 << 7)
#define BCREATE_HAS_WAYPOINT    (1 << 8)
#define BCREATE_HAS_PARTICLE    (1 << 9)
#define BCREATE_HAS_LIGHT       (1 << 10)
#define BCREATE_HAS_SOUND       (1 << 11)
#define BCREATE_HAS_AMMO        (1 << 12)
#define BCREATE_HAS_ANIMATION   (1 << 13)
#define BCREATE_HAS_DOOR        (1 << 14)
#define BCREATE_HAS_JOINT       (1 << 15)
#define BCREATE_HAS_VEHICLE     (1 << 16)
#define BCREATE_HAS_TEXT        (1 << 17)

// Serialize an entity's full component data into a binary buffer.
// Does NOT include the SnapBase header -- caller writes that separately.
// Returns number of bytes written, or 0 if buffer too small.
int binary_serialize_entity(uint8_t* buf, int buf_size, int entity);

// Deserialize an entity from binary component data.
// The entity must already exist (have a CoordinateComponent).
// Adds all components described in the binary data.
// Returns number of bytes consumed, or 0 on error.
int binary_deserialize_entity(const uint8_t* buf, int buf_size, int entity);

// Skip past binary entity data without creating anything.
// Parses the data to determine its size.
// Returns number of bytes consumed, or 0 on error.
int binary_skip_entity(const uint8_t* buf, int buf_size);
