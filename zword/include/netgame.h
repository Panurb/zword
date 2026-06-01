#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <SDL.h>

#include "component.h"
#include "game.h"
#include "network.h"

typedef struct {
    bool valid;
    char map_name[128];
    GameMode game_mode;
    int num_players;
    String player_names[NET_MAX_CLIENTS + 1];
    int point_limit;
} LobbyInfo;

#pragma pack(push, 1)

// Snapshot packet header: sent from host to clients every tick.
// Followed by variable-length entity data: for each entity,
// [entity_id(u16)] [binary_serialize_entity() output].
// After entity data: sound event count (u8) + sound events,
// then particle event count (u8) + particle events.
typedef struct {
    PacketHeader header;
    uint16_t entity_count;
    uint16_t wave;
    float wave_delay;
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
extern LobbyInfo cached_lobby_info;

// Host: build a snapshot of all dynamic entities into the send buffer.
// Returns the total packet size.
int netgame_build_snapshot(uint8_t* buf, int buf_size, uint32_t tick);

// Client: apply a received snapshot to the local game state.
void netgame_apply_snapshot(const uint8_t* buf, int size);

// Pack local player controller state into an InputPacket.
void netgame_pack_input(InputPacket* pkt, int player_entity, uint8_t player_slot, uint32_t tick);

// Client: update local controller state if needed and send one input packet.
bool netgame_client_send_input(bool neutral);

// Host: unpack a received input packet into a player entity's controller.
void netgame_unpack_input(const InputPacket* pkt, int player_entity);

void return_to_lobby();

bool netgame_is_dynamic(Entity entity);

bool netgame_should_sync(Entity entity);

void create_lobby(void);

void join_lobby(void);

void input_host(SDL_Event sdl_event);

void input_client(SDL_Event sdl_event);

void input_host_pause(SDL_Event sdl_event);

void input_client_pause(SDL_Event sdl_event);

void update_host_lobby(float time_step);

void update_client_lobby(float time_step);

void host_start(void);

void client_start(void);

void update_host(float time_step);

void update_client(float time_step);

void update_client_pause(float time_step);

void update_client_game_over(float time_step);

void draw_host_lobby(void);

void draw_client_lobby(void);
