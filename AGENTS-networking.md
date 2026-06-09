# AGENTS - Networking

Multiplayer is experimental. Host-authoritative client-server model over UDP. Assumes LAN environment (low latency, no packet loss). Network code uses `#ifdef _WIN32` to select between Winsock2 (Windows) and POSIX sockets (other platforms including Emscripten). There are no `__EMSCRIPTEN__` guards in the network module — socket calls compile on all platforms.

## Key files

| File | Role |
|------|------|
| `network.c/h` | Low-level UDP sockets (Winsock2 on Windows), send/receive, client connection management, timeouts |
| `netgame.c/h` | Game-level protocol: snapshot building/applying, input packing/unpacking, entity ID remapping, sound/particle event sync |
| `serialize_binary.c/h` | Compact binary serialization of entities/components for snapshots |
| `app.c` | Host loop (`STATE_HOST`), client loop (`STATE_CLIENT`) |

## Game states

Network play uses dedicated game states instead of checking `network.mode` at runtime:

| State | Purpose |
|-------|---------|
| `STATE_HOST` | Host game loop (receive inputs, simulate, broadcast snapshots) |
| `STATE_CLIENT` | Client game loop (send input, receive/apply snapshots) |
| `STATE_HOST_PAUSE` | Host pause menu |
| `STATE_CLIENT_PAUSE` | Client pause menu |
| `STATE_HOST_GAME_OVER` | Host-only transition state for returning the current match to lobby |
| `STATE_CLIENT_GAME_OVER` | Client game-over screen while waiting for or reacting to host lobby return |

`STATE_START` routes to the correct state based on `network.mode` and creates the appropriate pause menu (`create_host_pause_menu()`, `create_client_pause_menu()`, or `create_pause_menu()` for single-player). The pause menu functions live in `menu.c` — `menu.c` does not include `network.h`.

`STATE_GAME` / `STATE_PAUSE` are single-player only.

## Protocol

- Transport: UDP, non-blocking sockets
- Default port: 12345, max 3 clients (4 players total)
- Disconnect timeout: 2 seconds
- Max packet size: 65000 bytes
- CLI: `--host [port]` or `--join [ip] [port]`
- For local prediction testing, `netgame.c` can buffer snapshots on the client via `NET_BUFFERED_SNAPSHOT_COUNT`; `0` disables buffering, and when the FIFO buffer is full the client applies the oldest buffered snapshot

Packet types:

| Type | Direction | Purpose |
|------|-----------|---------|
| `PACKET_JOIN` | Client -> Host | Request to join |
| `PACKET_JOIN_ACK` | Host -> Client | Accept with player slot assignment |
| `PACKET_KEEPALIVE` | Client -> Host | Lobby heartbeat so the host can timeout disconnected clients |
| `PACKET_INPUT` | Client -> Host | Controller state (sticks, triggers, button flags) |
| `PACKET_SNAPSHOT` | Host -> Clients | Full game state (binary-serialized entities + sound/particle events) |
| `PACKET_START_GAME` | Host -> Clients | Map name, game mode, player count |
| `PACKET_END_GAME` | Host -> Clients | Match-end result (`MATCH_END_GAME_OVER` or `MATCH_END_WIN`) |
| `PACKET_LOBBY_INFO` | Host -> Clients | Current lobby map, mode, and connected player names |
| `PACKET_DISCOVER` | Browser -> Host | LAN discovery query sent by the server browser to `NET_DEFAULT_PORT` |
| `PACKET_DISCOVER_RESPONSE` | Host -> Browser | LAN discovery response with host IP/port, host name, and player counts |

## Snapshot format

The snapshot packet layout is:

1. `SnapshotPacket` header (includes `entity_count`)
2. Entity data: N × `[u16 entity_id][binary_serialize_entity() output]`
3. Sound events: `[u8 count]` + N × `[u16 entity_id][u16 sound_index][u8 volume]` (5 bytes each)
4. Particle events: `[u8 count]` + N × `[u16 entity_id][u16 count][float origin_x][float origin_y][float max_time]` (16 bytes each)

Player runtime data inside snapshots includes `kills` and `deaths`, so client HUD / end-game UI can read authoritative deathmatch stats directly from `PlayerComponent`.

Sound events are discovered at snapshot-build time by scanning `SoundComponent.events[]` for pending one-shot events. Particle events use `ParticleComponent.pending_burst`, which accumulates burst counts across multiple hits in one frame (e.g. shotgun pellets). Only non-looping particles are synced; looping emitters run locally on clients.

## Game loops

### Host (`app.c`, `STATE_HOST`)

Each tick at 60 Hz:
1. Receive all queued input packets, apply to remote player controllers
2. Check client timeouts
3. Run `input_players()` -- local controller update + state machines for all players
4. Simulate (`update_game()`, `update_game_mode()`)
5. Build binary snapshot (entities + sound events + particle events), broadcast to all clients

### Host Lobby (`app.c`, `STATE_HOST_LOBBY`)

Each tick:
1. Receive join and keepalive packets from clients
2. Check client timeouts
3. Rebuild active player controllers from connected clients
4. Periodically broadcast `PACKET_LOBBY_INFO`

### Client (`app.c`, `STATE_CLIENT`)

Each tick:
1. `update_controller()` for local player only (no state machine -- host is authoritative)
2. Pack and send `InputPacket` to host
3. Save current positions as `previous` (for interpolation)
4. Receive snapshots immediately; when snapshot buffering is enabled, enqueue them in a client-side FIFO
5. When the FIFO buffer reaches `NET_BUFFERED_SNAPSHOT_COUNT`, apply and remove the oldest buffered snapshot (overwrites current entity state, replays sound events, triggers particle bursts)
6. Rebuild collision grid from the authoritative snapshot
7. Reconcile the local player by replaying buffered local inputs newer than the snapshot tick; prediction covers on-foot movement and aiming plus local hitscan weapon muzzle feedback (shot sound and tracer particles only), and reuses collision response against the latest snapshot world without running trigger side effects
8. Timeout back to menu if the host has been silent for more than `NET_DISCONNECT_TIMEOUT`
9. Update camera, lights, particles locally

### Client Lobby (`app.c`, `STATE_CLIENT_LOBBY`)

Each tick:
1. Periodically send `PACKET_KEEPALIVE` to the host
2. Receive `PACKET_JOIN_ACK`, `PACKET_LOBBY_INFO`, and `PACKET_START_GAME`
3. Timeout back to menu if the host has been silent for more than `NET_DISCONNECT_TIMEOUT`
4. Update the lobby UI from the latest cached lobby info

### Server Browser

1. Browser sends one UDP broadcast `PACKET_DISCOVER` to `255.255.255.255:12345`
2. Hosts listening on `NET_DEFAULT_PORT` reply directly to the sender with `PACKET_DISCOVER_RESPONSE`
3. Browser collects responses for a short window and deduplicates by source IP and advertised port
4. Clicking a browser row copies the discovered host IP into the LAN join textbox and uses the normal `PACKET_JOIN` flow

## Match end flow

- Host ending a LAN match does not use `STATE_END`, because that tears down the session and recreates the main menu.
- Instead, the host enters `STATE_HOST_GAME_OVER` and periodically broadcasts `PACKET_END_GAME` while the game-over / win screen is visible.
- Clients handle `PACKET_END_GAME` in active gameplay states and enter `STATE_CLIENT_GAME_OVER` immediately, recreating the correct match-end menu from the packet result.
- When the host leaves the match-end screen, it tears down the match world with `end_match()`, rebuilds the host lobby UI, and resumes broadcasting `PACKET_LOBBY_INFO`.
- Clients cache `PACKET_LOBBY_INFO` for the lobby UI and also treat it as the signal that the host has returned from a match back to lobby.
- The host rebroadcasts `PACKET_LOBBY_INFO` periodically while in `STATE_HOST_LOBBY`, so clients can recover from dropped UDP packets and keep their lobby UI in sync.
- Client-side host timeout checks run in lobby, active gameplay, and client pause. They intentionally do not run in `STATE_CLIENT_GAME_OVER`, because the host does not periodically send packets during `STATE_HOST_GAME_OVER`.

## Input handling

Local players: `update_controller()` (`input.c`) computes `buttons_pressed` as a rising edge from `buttons_down`. The flag is `true` for exactly one tick, then cleared by the next call.

Remote players on host: `update_controller()` is skipped. Flags are set by `netgame_unpack_input()` from network packets. Edge-triggered flags (`buttons_pressed`, `buttons_released`) are OR'd across packets within a tick to prevent overwrite, then explicitly cleared after the state machine runs (`input.c`, end of `input_players()`).

Clients also keep a small ring buffer of their own sent controller states, keyed by `PacketHeader.tick`. After each snapshot, the client discards acknowledged inputs and replays newer buffered inputs for the local player only.

## Entity ID remapping

Clients may allocate entities in different order than the host. Two mapping arrays handle this:
- `net_host_to_local[host_id]` -> local entity ID
- `net_local_to_host[local_id]` -> host entity ID

Snapshots are serialized with host IDs. `netgame_apply_snapshot()` remaps on the client side, creating local entities as needed.

The snapshot header can also carry small pieces of authoritative match-level state that do not belong to any entity. Survival currently syncs `wave` and `wave_delay` this way so client-side HUD and end-game text match the host.

## Known limitations

- **Prediction scope is narrow** -- only the local player's on-foot movement, aiming, and hitscan muzzle feedback are predicted; damage, blood/hit effects, vehicles, interactions, and world state remain fully host-authoritative
- **Snapshot size cap** -- 65 KB limit can cause entities to be omitted from snapshots
- **Pause is local only** -- pausing on the host or client only pauses locally; the other side keeps running
- **Lobby info is best-effort UDP** -- periodic rebroadcast helps, but there is still no full ack/retry reliability layer
- **Lobby disconnects are timeout-based** -- the host removes lobby clients after missed keepalives, so disconnect detection is not immediate
- **LAN discovery is subnet-local** -- it uses IPv4 UDP broadcast to `255.255.255.255`, so routers typically will not forward it beyond the local subnet
