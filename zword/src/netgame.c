#include <stdio.h>
#include <string.h>

#include "app.h"
#include "camera.h"
#include "light.h"
#include "menu.h"
#include "netgame.h"
#include "network.h"
#include "component.h"
#include "game.h"
#include "grid.h"
#include "health.h"
#include "input.h"
#include "settings.h"
#include "serialize_binary.h"
#include "sound.h"
#include "particle.h"

bool net_entity_seen[MAX_ENTITIES];

static float lobby_info_broadcast_timer = 0.0f;

static float lobby_keepalive_timer = 0.0f;

LobbyInfo cached_lobby_info = {0};

DiscoveredServer discovered_servers[NET_MAX_CLIENTS + 1] = {0};

int discovered_servers_count = 0;


static void cache_lobby_info(const LobbyInfoPacket* lobby) {
    cached_lobby_info.valid = true;
    strncpy(cached_lobby_info.map_name, lobby->map_name, sizeof(cached_lobby_info.map_name) - 1);
    cached_lobby_info.map_name[sizeof(cached_lobby_info.map_name) - 1] = '\0';
    cached_lobby_info.game_mode = (GameMode)lobby->game_mode;
    cached_lobby_info.num_players = lobby->num_players;
    cached_lobby_info.friendly_fire = lobby->friendly_fire;
    cached_lobby_info.point_limit = lobby->point_limit;

    for (int i = 0; i < NET_MAX_CLIENTS + 1; i++) {
        strncpy(cached_lobby_info.player_names[i], lobby->player_names[i], sizeof(cached_lobby_info.player_names[i]) - 1);
        cached_lobby_info.player_names[i][sizeof(cached_lobby_info.player_names[i]) - 1] = '\0';
    }
}


void netgame_clear_discovered_servers(void) {
    memset(discovered_servers, 0, sizeof(discovered_servers));
    discovered_servers_count = 0;
}


void netgame_store_discovered_server(const DiscoverResponsePacket* pkt, const struct sockaddr_in* from_addr) {
    if (pkt->num_players <= 0 || pkt->max_players <= 0) {
        return;
    }

    char host_ip[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &from_addr->sin_addr, host_ip, sizeof(host_ip)) == NULL) {
        return;
    }

    for (int i = 0; i < discovered_servers_count; i++) {
        if (strcmp(discovered_servers[i].host_ip, host_ip) != 0) {
            continue;
        }
        if (discovered_servers[i].port != pkt->port) {
            continue;
        }

        discovered_servers[i].valid = true;
        strcpy(discovered_servers[i].host_name, pkt->host_name);
        discovered_servers[i].num_players = pkt->num_players;
        discovered_servers[i].max_players = pkt->max_players;
        return;
    }

    if (discovered_servers_count >= LENGTH(discovered_servers)) {
        return;
    }

    DiscoveredServer* server = &discovered_servers[discovered_servers_count++];
    memset(server, 0, sizeof(*server));
    server->valid = true;
    strcpy(server->host_ip, host_ip);
    server->port = pkt->port;
    strcpy(server->host_name, pkt->host_name);
    server->num_players = pkt->num_players;
    server->max_players = pkt->max_players;
}


static void build_lobby_info_packet(LobbyInfoPacket* pkt) {
    memset(pkt, 0, sizeof(*pkt));
    pkt->header.type = PACKET_LOBBY_INFO;
    pkt->header.tick = network.tick;
    pkt->header.size = sizeof(LobbyInfoPacket);
    strncpy(pkt->map_name, game_data->map_name, sizeof(pkt->map_name) - 1);
    pkt->game_mode = (uint8_t)game_data->game_mode;
    pkt->num_players = 1;
    pkt->friendly_fire = game_data->friendly_fire ? 1 : 0;
    pkt->point_limit = game_data->point_limit;

    strncpy(pkt->player_names[0], game_settings.player_name, sizeof(pkt->player_names[0]) - 1);
    for (int i = 0; i < NET_MAX_CLIENTS; i++) {
        if (!network.clients[i].connected) continue;

        pkt->num_players++;
        strncpy(pkt->player_names[i + 1], network.clients[i].player_name, sizeof(pkt->player_names[i + 1]) - 1);
    }
}


static void broadcast_lobby_info() {
    LobbyInfoPacket pkt;
    build_lobby_info_packet(&pkt);
    cache_lobby_info(&pkt);
    network_broadcast(&pkt, sizeof(pkt));
}


static void send_lobby_keepalive() {
    if (network.mode != NET_MODE_CLIENT) {
        return;
    }

    KeepAlivePacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.header.type = PACKET_KEEPALIVE;
    pkt.header.tick = network.tick;
    pkt.header.size = sizeof(KeepAlivePacket);
    network_send_to_host(&pkt, sizeof(pkt));
}


static void mark_server_packet_received() {
    if (network.mode != NET_MODE_CLIENT) {
        return;
    }

    network.last_server_recv_time = SDL_GetTicks() / 1000.0f;
}


static bool client_check_server_timeout() {
    if (network.mode != NET_MODE_CLIENT) {
        return false;
    }

    if (network.last_server_recv_time <= 0.0f) {
        return false;
    }

    float current_time = SDL_GetTicks() / 1000.0f;
    float elapsed = current_time - network.last_server_recv_time;
    if (elapsed <= NET_DISCONNECT_TIMEOUT) {
        return false;
    }

    LOG_WARNING("Host timed out (%.1fs)", elapsed);
    game_state = STATE_END;
    return true;
}


static void enter_client_match_end(const EndGamePacket* pkt) {
    if (pkt->end_type == MATCH_END_WIN) {
        enter_match_end_screen(true);
    } else {
        enter_match_end_screen(false);
    }
}


static void reset_host_client_timeouts() {
    if (network.mode != NET_MODE_HOST) {
        return;
    }

    for (int i = 0; i < NET_MAX_CLIENTS; i++) {
        if (!network.clients[i].connected) continue;

        network.clients[i].last_recv_time = 0.0f;
    }
}


static void rebuild_host_player_controllers() {
    if (network.mode != NET_MODE_HOST) {
        return;
    }

    for (int i = 0; i < 4; i++) {
        app.player_controllers[i] = CONTROLLER_NONE;
    }

    app.player_controllers[0] = CONTROLLER_MKB;
    for (int i = 0; i < NET_MAX_CLIENTS; i++) {
        if (!network.clients[i].connected) continue;

        int slot = network.clients[i].player_slot;
        if (slot > 0 && slot < 4) {
            app.player_controllers[slot] = CONTROLLER_MKB;
        }
    }
}


static bool client_receive_packets(void) {
    struct sockaddr_in from;
    int received;
    while ((received = network_receive(network.recv_buf, NET_MAX_PACKET_SIZE, &from)) > 0) {
        if (received < (int)sizeof(PacketHeader)) continue;

        PacketHeader* hdr = (PacketHeader*)network.recv_buf;
        if (hdr->type == PACKET_SNAPSHOT) {
            mark_server_packet_received();
            netgame_apply_snapshot(network.recv_buf, received);
            return false;
        }

        if (hdr->type == PACKET_END_GAME && received >= (int)sizeof(EndGamePacket)) {
            mark_server_packet_received();
            enter_client_match_end((EndGamePacket*)network.recv_buf);
            return true;
        }

        if (hdr->type == PACKET_LOBBY_INFO && received >= (int)sizeof(LobbyInfoPacket)) {
            mark_server_packet_received();
            cache_lobby_info((LobbyInfoPacket*)network.recv_buf);
            return_to_lobby();
            return true;
        }
    }

    return false;
}


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


void return_to_lobby() {
    end_match();
    clear_all_sounds();
    network.game_started = false;
    lobby_info_broadcast_timer = 0.0f;
    lobby_keepalive_timer = 0.0f;
    reset_host_client_timeouts();
    rebuild_host_player_controllers();

    destroy_menu();
    if (network.mode == NET_MODE_HOST) {
        create_host_lobby_menu();
        game_state = STATE_HOST_LOBBY;
    } else if (network.mode == NET_MODE_CLIENT) {
        create_lobby_menu();
        game_state = STATE_CLIENT_LOBBY;
    } else {
        create_menu();
        game_state = STATE_MENU;
    }
}


bool netgame_is_dynamic(Entity entity) {
    // Check own components
    if (has_dynamic_components(entity)) return true;

    // Walk parent chain: if any ancestor is dynamic, so are we
    CoordinateComponent* coord = CoordinateComponent_get(entity);
    if (coord && coord->parent >= 0) {
        return netgame_is_dynamic(coord->parent);
    }
    return false;
}


bool netgame_should_sync(Entity entity) {
    if (netgame_is_dynamic(entity)) return true;

    ImageComponent* image = ImageComponent_get(entity);
    if (image && fabsf(image->stretch) > 1e-3f) {
        return true;
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

        if (!netgame_should_sync(i)) continue;

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
            return;
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


void create_lobby(void) {
    if (!network_host_start(NET_DEFAULT_PORT)) {
        LOG_ERROR("Failed to start host");
        game_state = STATE_MENU;
    } else {
        rebuild_host_player_controllers();
        destroy_menu();
        create_host_lobby_menu();
        game_state = STATE_HOST_LOBBY;
    }
}


void join_lobby(void) {
    if (!network_client_connect(network.host_ip, NET_DEFAULT_PORT)) {
        LOG_ERROR("Failed to connect to host");
        game_state = STATE_MENU;
    } else {
        destroy_menu();
        create_lobby_menu();
        game_state = STATE_CLIENT_LOBBY;
    }
}


void input_host(SDL_Event sdl_event) {
    if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.repeat == 0) {
        if (sdl_event.key.keysym.sym == SDLK_ESCAPE || sdl_event.key.keysym.sym == SDLK_p) {
            game_state = STATE_HOST_PAUSE;
        }
    }
    if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_TAB) {
        game_data->show_leaderboard = true;
    } else if (sdl_event.type == SDL_KEYUP && sdl_event.key.keysym.sym == SDLK_TAB) {
        game_data->show_leaderboard = false;
    }
}


void input_client(SDL_Event sdl_event) {
    if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.repeat == 0) {
        if (sdl_event.key.keysym.sym == SDLK_ESCAPE || sdl_event.key.keysym.sym == SDLK_p) {
            game_state = STATE_CLIENT_PAUSE;
        }
    }
    if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_TAB) {
        game_data->show_leaderboard = true;
    } else if (sdl_event.type == SDL_KEYUP && sdl_event.key.keysym.sym == SDLK_TAB) {
        game_data->show_leaderboard = false;
    }
}


void input_host_pause(SDL_Event sdl_event) {
    input_menu(game_data->menu_camera, sdl_event);
}


void input_client_pause(SDL_Event sdl_event) {
    input_menu(game_data->menu_camera, sdl_event);
}


void update_host_lobby(float time_step) {
    network_host_accept_clients();
    network_check_timeouts(SDL_GetTicks() / 1000.0f);
    rebuild_host_player_controllers();
    if (lobby_info_broadcast_timer <= 0.0f) {
        broadcast_lobby_info();
        lobby_info_broadcast_timer = 0.25f;
    } else {
        lobby_info_broadcast_timer = fmaxf(lobby_info_broadcast_timer - time_step, 0.0f);
    }

    update_menu(time_step);
}


void update_client_lobby(float time_step) {
    if (lobby_keepalive_timer <= 0.0f) {
        send_lobby_keepalive();
        lobby_keepalive_timer = 0.25f;
    } else {
        lobby_keepalive_timer = fmaxf(lobby_keepalive_timer - time_step, 0.0f);
    }

    struct sockaddr_in from;
    int received;
    while ((received = network_receive(network.recv_buf, NET_MAX_PACKET_SIZE, &from)) > 0) {
        if (received < (int)sizeof(PacketHeader)) continue;

        PacketHeader* hdr = (PacketHeader*)network.recv_buf;
        if (hdr->type == PACKET_JOIN_ACK && received >= (int)sizeof(JoinAckPacket)) {
            mark_server_packet_received();
            JoinAckPacket* ack = (JoinAckPacket*)network.recv_buf;
            network.local_player_slot = ack->player_slot;
            LOG_INFO("Joined as player %d", network.local_player_slot);
        } else if (hdr->type == PACKET_LOBBY_INFO && received >= (int)sizeof(LobbyInfoPacket)) {
            mark_server_packet_received();
            cache_lobby_info((LobbyInfoPacket*)network.recv_buf);
        } else if (hdr->type == PACKET_START_GAME && received >= (int)sizeof(StartGamePacket)) {
            mark_server_packet_received();
            StartGamePacket* start = (StartGamePacket*)network.recv_buf;
            strncpy(game_data->map_name, start->map_name, sizeof(game_data->map_name) - 1);
            game_data->map_name[sizeof(game_data->map_name) - 1] = '\0';
            game_data->game_mode = (GameMode)start->game_mode;
            game_data->point_limit = start->point_limit;

            for (int i = 0; i < 4; i++) {
                app.player_controllers[i] = CONTROLLER_NONE;
            }
            for (int i = 0; i < (int)start->num_players; i++) {
                app.player_controllers[i] = CONTROLLER_MKB;
            }
            network.game_started = true;
            lobby_info_broadcast_timer = 0.0f;
            game_state = STATE_CLIENT_START;
            break;
        }
    }

    if (client_check_server_timeout()) {
        return;
    }

    update_menu(time_step);
}


void host_start(void) {
    rebuild_host_player_controllers();
    start_game(game_data->map_name, false);
    set_player_names(cached_lobby_info.player_names, cached_lobby_info.num_players);
    reset_host_client_timeouts();

    int num_players = 1;
    for (int i = 0; i < NET_MAX_CLIENTS; i++) {
        if (network.clients[i].connected) {
            num_players++;
        }
    }

    StartGamePacket start_pkt;
    memset(&start_pkt, 0, sizeof(start_pkt));
    start_pkt.header.type = PACKET_START_GAME;
    start_pkt.header.tick = network.tick;
    start_pkt.header.size = sizeof(StartGamePacket);
    strncpy(start_pkt.map_name, game_data->map_name, sizeof(start_pkt.map_name) - 1);
    start_pkt.game_mode = (uint8_t)game_data->game_mode;
    start_pkt.num_players = (uint8_t)num_players;
    start_pkt.point_limit = (uint8_t)game_data->point_limit;
    network_broadcast(&start_pkt, sizeof(start_pkt));
    network.game_started = true;
    lobby_info_broadcast_timer = 0.0f;

    memset(net_entity_seen, 0, sizeof(net_entity_seen));

    create_host_pause_menu();
    game_state = STATE_HOST;
}


void client_start(void) {
    start_game(game_data->map_name, false);
    set_player_names(cached_lobby_info.player_names, cached_lobby_info.num_players);

    memset(net_entity_seen, 0, sizeof(net_entity_seen));

    create_client_pause_menu();
    game_state = STATE_CLIENT;
}


void update_host(float time_step, bool paused) {
    float current_time = SDL_GetTicks() / 1000.0f;

    struct sockaddr_in from;
    int received;
    while ((received = network_receive(network.recv_buf, NET_MAX_PACKET_SIZE, &from)) > 0) {
        if (received < (int)sizeof(PacketHeader)) continue;

        PacketHeader* hdr = (PacketHeader*)network.recv_buf;
        if (hdr->type == PACKET_INPUT && received >= (int)sizeof(InputPacket)) {
            InputPacket* input_pkt = (InputPacket*)network.recv_buf;
            for (int ci = 0; ci < NET_MAX_CLIENTS; ci++) {
                if (network.clients[ci].connected &&
                    from.sin_addr.s_addr == network.clients[ci].addr.sin_addr.s_addr &&
                    from.sin_port == network.clients[ci].addr.sin_port) {
                    network.clients[ci].last_recv_time = current_time;
                    break;
                }
            }

            int slot_idx = 0;
            ListNode* pnode;
            FOREACH(pnode, game_data->components->player.order) {
                if (slot_idx == (int)input_pkt->player_slot) {
                    netgame_unpack_input(input_pkt, pnode->value);
                    break;
                }
                slot_idx++;
            }
        }
    }

    int disconnected = network_check_timeouts(current_time);
    if (disconnected) {
        int i = 0;
        ListNode* node;
        FOREACH(node, game_data->components->player.order) {
            if (disconnected & (1 << i)) {
                die(node->value);
            }
            i++;
        }
    }

    input_players(game_data->camera, !paused);
    update_game(time_step);
    update_game_mode(time_step);

    if (game_state == STATE_HOST_LOBBY) {
        return_to_lobby();
        return;
    }

    int snap_size = netgame_build_snapshot(network.send_buf, NET_MAX_PACKET_SIZE, network.tick);
    network_broadcast(network.send_buf, snap_size);
    network.tick++;
}


void update_client(float time_step) {
    netgame_client_send_input(false);

    update_coordinates();

    if (client_receive_packets()) {
        return;
    }

    if (client_check_server_timeout()) {
        return;
    }

    ColliderGrid_clear(game_data->grid);
    init_grid();
    update_camera(game_data->camera, time_step, true);
    update_lights(time_step);
    update_particles(game_data->camera, time_step);

    network.tick++;
}


void update_client_pause(float time_step) {
    netgame_client_send_input(true);

    update_coordinates();

    if (client_receive_packets()) {
        return;
    }

    if (client_check_server_timeout()) {
        return;
    }

    ColliderGrid_clear(game_data->grid);
    init_grid();
    update_camera(game_data->camera, time_step, true);
    update_lights(time_step);
    update_particles(game_data->camera, time_step);

    CoordinateComponent* cam_coord = CoordinateComponent_get(game_data->camera);
    cam_coord->previous.position = cam_coord->position;
    cam_coord->previous.angle = cam_coord->angle;
    cam_coord->previous.scale = cam_coord->scale;

    network.tick++;
    update_menu(time_step);
}


void update_client_game_over(float time_step) {
    if (client_receive_packets()) {
        return;
    }

    update_game_over(time_step);
}
