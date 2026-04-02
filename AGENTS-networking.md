# AGENTS - Networking

Multiplayer is experimental. Host-authoritative client-server model over UDP. Assumes LAN environment (low latency, no packet loss). Networking libraries (Winsock2) are unavailable on Emscripten, so socket code is guarded with `#ifndef __EMSCRIPTEN__`.

## Key files

| File | Role |
|------|------|
| `network.c/h` | Low-level UDP sockets (Winsock2 on Windows), send/receive, client connection management, timeouts |
| `netgame.c/h` | Game-level protocol: snapshot building/applying, input packing/unpacking, entity ID remapping, sound/particle event sync |
| `serialize_binary.c/h` | Compact binary serialization of entities/components for snapshots |
| `app.c` | Host loop (`STATE_HOST`, ~line 486), client loop (`STATE_CLIENT`, ~line 565) |

## Game states

Network play uses dedicated game states instead of checking `network.mode` at runtime:

| State | Purpose |
|-------|---------|
| `STATE_HOST` | Host game loop (receive inputs, simulate, broadcast snapshots) |
| `STATE_CLIENT` | Client game loop (send input, receive/apply snapshots) |
| `STATE_HOST_PAUSE` | Host pause menu |
| `STATE_CLIENT_PAUSE` | Client pause menu |

`STATE_START` routes to the correct state based on `network.mode` and creates the appropriate pause menu (`create_host_pause_menu()`, `create_client_pause_menu()`, or `create_pause_menu()` for single-player). The pause menu functions live in `menu.c` — `menu.c` does not include `network.h`.

`STATE_GAME` / `STATE_PAUSE` are single-player only.

## Protocol

- Transport: UDP, non-blocking sockets
- Default port: 12345, max 3 clients (4 players total)
- Disconnect timeout: 5 seconds
- Max packet size: 65000 bytes
- CLI: `--host [port]` or `--join [ip] [port]`

Packet types:

| Type | Direction | Purpose |
|------|-----------|---------|
| `PACKET_JOIN` | Client -> Host | Request to join |
| `PACKET_JOIN_ACK` | Host -> Client | Accept with player slot assignment |
| `PACKET_INPUT` | Client -> Host | Controller state (sticks, triggers, button flags) |
| `PACKET_SNAPSHOT` | Host -> Clients | Full game state (binary-serialized entities + sound/particle events) |
| `PACKET_START_GAME` | Host -> Clients | Map name, game mode, player count |

## Snapshot format

The snapshot packet layout is:

1. `SnapshotPacket` header (includes `entity_count`)
2. Entity data: N × `[u16 entity_id][binary_serialize_entity() output]`
3. Sound events: `[u8 count]` + N × `[u16 entity_id][u16 sound_index][u8 volume]` (5 bytes each)
4. Particle events: `[u8 count]` + N × `[u16 entity_id][u16 count][float origin_x][float origin_y][float max_time]` (16 bytes each)

Sound events are discovered at snapshot-build time by scanning `SoundComponent.events[]` for pending one-shot events. Particle events use `ParticleComponent.pending_burst`, which accumulates burst counts across multiple hits in one frame (e.g. shotgun pellets). Only non-looping particles are synced; looping emitters run locally on clients.

## Game loops

### Host (`app.c`, `STATE_HOST`, ~line 486)

Each tick at 60 Hz:
1. Receive all queued input packets, apply to remote player controllers
2. Check client timeouts
3. Run `input_players()` -- local controller update + state machines for all players
4. Simulate (`update_game()`, `update_game_mode()`)
5. Build binary snapshot (entities + sound events + particle events), broadcast to all clients

### Client (`app.c`, `STATE_CLIENT`, ~line 565)

Each tick:
1. `update_controller()` for local player only (no state machine -- host is authoritative)
2. Pack and send `InputPacket` to host
3. Save current positions as `previous` (for interpolation)
4. Receive and apply snapshots (overwrites current entity state, replays sound events, triggers particle bursts)
5. Rebuild collision grid (needed for light raycasting)
6. Update camera, lights, particles locally

## Input handling

Local players: `update_controller()` (`input.c:252-254`) computes `buttons_pressed` as a rising edge from `buttons_down`. The flag is `true` for exactly one tick, then cleared by the next call.

Remote players on host: `update_controller()` is skipped. Flags are set by `netgame_unpack_input()` from network packets. Edge-triggered flags (`buttons_pressed`, `buttons_released`) are OR'd across packets within a tick to prevent overwrite, then explicitly cleared after the state machine runs (`input.c`, end of `input_players()`).

## Entity ID remapping

Clients may allocate entities in different order than the host. Two mapping arrays handle this:
- `net_host_to_local[host_id]` -> local entity ID
- `net_local_to_host[local_id]` -> host entity ID

Snapshots are serialized with host IDs. `netgame_apply_snapshot()` remaps on the client side, creating local entities as needed.

## Known limitations

- **No client-side prediction** -- all actions have a full round-trip delay before the client sees the result
- **Snapshot size cap** -- 65 KB limit can cause entities to be omitted from snapshots
- **Pause is local only** -- pausing on the host or client only pauses locally; the other side keeps running
