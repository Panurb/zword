# AGENTS - Networking

Multiplayer is experimental. Host-authoritative client-server model over UDP. Assumes LAN environment (low latency, no packet loss). Networking libraries (Winsock2) are unavailable on Emscripten, so socket code is guarded with `#ifndef __EMSCRIPTEN__`.

## Key files

| File | Role |
|------|------|
| `network.c/h` | Low-level UDP sockets (Winsock2 on Windows), send/receive, client connection management, timeouts |
| `netgame.c/h` | Game-level protocol: snapshot building/applying, input packing/unpacking, entity ID remapping |
| `serialize_binary.c/h` | Compact binary serialization of entities/components for snapshots |
| `app.c` | Host and client game loops (lines ~440-580) |

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
| `PACKET_SNAPSHOT` | Host -> Clients | Full game state (binary-serialized entities) |
| `PACKET_START_GAME` | Host -> Clients | Map name, game mode, player count |

## Game loops

### Host (`app.c`, ~line 441)

Each tick at 60 Hz:
1. Receive all queued input packets, apply to remote player controllers
2. Check client timeouts
3. Run `input_players()` -- local controller update + state machines for all players
4. Simulate (`update_game()`, `update_game_mode()`)
5. Build binary snapshot, broadcast to all clients

### Client (`app.c`, ~line 517)

Each tick:
1. `update_controller()` for local player only (no state machine -- host is authoritative)
2. Pack and send `InputPacket` to host
3. Save current positions as `previous` (for interpolation)
4. Receive and apply snapshots (overwrites current entity state)
5. Update camera

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
- **No sound sync** -- sound events triggered on the host are not transmitted to clients
- **Snapshot size cap** -- 65 KB limit can cause entities to be omitted from snapshots
