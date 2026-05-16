#include <stdio.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>

#include "app.h"
#include "game.h"
#include "settings.h"
#include "interface.h"
#include "player.h"
#include "menu.h"
#include "editor.h"
#include "hud.h"
#include "input.h"
#include "serialize.h"
#include "list.h"
#include "camera.h"
#include "light.h"
#include "grid.h"
#include "health.h"
#include "particle.h"
#include "network.h"
#include "netgame.h"
#ifdef __EMSCRIPTEN__
    #include <emscripten/html5.h>
#endif


App app;

static GameState previous_state = STATE_MENU;

static String version = "0.4.1";

static float title_scale = 2.0f;

static float lobby_info_broadcast_timer = 0.0f;

static int debug_level = 0;

static bool show_leaderboard = false;

typedef struct {
    int page;
    int panel;
    Vector2f position;
    float scale;
} Intro;

typedef struct {
    bool valid;
    char map_name[128];
    GameMode game_mode;
    int num_players;
    String player_names[NET_MAX_CLIENTS + 1];
    int point_limit;
} LobbyInfo;

static Intro intro = {1, 0, {0.0f, 0.0f}, 3.0f};

static LobbyInfo cached_lobby_info = {0};


static const char* game_mode_to_string(GameMode mode) {
    switch (mode) {
        case MODE_SURVIVAL:
            return "Survival";
        case MODE_CAMPAIGN:
            return "Campaign";
        case MODE_TUTORIAL:
            return "Tutorial";
        case MODE_DEATHMATCH:
            return "Deathmatch";
    }

    return "Unknown";
}


static void cache_lobby_info(const LobbyInfoPacket* lobby) {
    cached_lobby_info.valid = true;
    strncpy(cached_lobby_info.map_name, lobby->map_name, sizeof(cached_lobby_info.map_name) - 1);
    cached_lobby_info.map_name[sizeof(cached_lobby_info.map_name) - 1] = '\0';
    cached_lobby_info.game_mode = (GameMode)lobby->game_mode;
    cached_lobby_info.num_players = lobby->num_players;
    cached_lobby_info.point_limit = lobby->point_limit;

    for (int i = 0; i < NET_MAX_CLIENTS + 1; i++) {
        strncpy(cached_lobby_info.player_names[i], lobby->player_names[i], sizeof(cached_lobby_info.player_names[i]) - 1);
        cached_lobby_info.player_names[i][sizeof(cached_lobby_info.player_names[i]) - 1] = '\0';
    }
}


static void build_lobby_info_packet(LobbyInfoPacket* pkt) {
    memset(pkt, 0, sizeof(*pkt));
    pkt->header.type = PACKET_LOBBY_INFO;
    pkt->header.tick = network.tick;
    pkt->header.size = sizeof(LobbyInfoPacket);
    strncpy(pkt->map_name, game_data->map_name, sizeof(pkt->map_name) - 1);
    pkt->game_mode = (uint8_t)game_data->game_mode;
    pkt->num_players = 1;
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


static void return_to_lobby() {
    end_match();
    clear_all_sounds();
    network.game_started = false;
    lobby_info_broadcast_timer = 0.0f;
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


static bool client_receive_packets(void) {
    struct sockaddr_in from;
    int received;
    while ((received = network_receive(network.recv_buf, NET_MAX_PACKET_SIZE, &from)) > 0) {
        if (received < (int)sizeof(PacketHeader)) continue;

        PacketHeader* hdr = (PacketHeader*)network.recv_buf;
        if (hdr->type == PACKET_SNAPSHOT) {
            netgame_apply_snapshot(network.recv_buf, received);
        } else if (hdr->type == PACKET_END_GAME && received >= (int)sizeof(EndGamePacket)) {
            enter_client_match_end((EndGamePacket*)network.recv_buf);
            return true;
        } else if (hdr->type == PACKET_LOBBY_INFO && received >= (int)sizeof(LobbyInfoPacket)) {
            cache_lobby_info((LobbyInfoPacket*)network.recv_buf);
            return_to_lobby();
            return true;
        }
    }

    return false;
}


void create_screen_textures() {
    SDL_PixelFormatEnum pixel_format = SDL_PIXELFORMAT_ABGR8888;

    app.shadow_texture = SDL_CreateTexture(app.renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, 
        game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.shadow_texture, SDL_BLENDMODE_BLEND);

    app.light_texture = SDL_CreateTexture(app.renderer, pixel_format, SDL_TEXTUREACCESS_TARGET, 
        game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.light_texture, SDL_BLENDMODE_MUL);

    app.blood_texture = SDL_CreateTexture(app.renderer, pixel_format, SDL_TEXTUREACCESS_TARGET,
        game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.blood_texture, SDL_BLENDMODE_BLEND);

    int threshold = 32;
    app.blood_threshold_texture = SDL_CreateTexture(app.renderer, pixel_format, SDL_TEXTUREACCESS_TARGET,
        game_settings.width, game_settings.height);
    SDL_SetRenderTarget(app.renderer, app.blood_threshold_texture);
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255 - threshold);
    SDL_RenderClear(app.renderer);
    SDL_SetRenderTarget(app.renderer, NULL);

    SDL_BlendMode bm = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, 
        SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD
    );
    SDL_SetTextureBlendMode(app.blood_threshold_texture, bm);

    app.blood_multiply_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
        game_settings.width, game_settings.height);

    bm = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_ONE, SDL_BLENDOPERATION_ADD, 
        SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_DST_ALPHA, SDL_BLENDOPERATION_ADD
    );
    SDL_SetTextureBlendMode(app.blood_multiply_texture, bm);
}


void create_game_window() {
    app.window = SDL_CreateWindow("NotK", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        game_settings.width, game_settings.height, SDL_WINDOW_SHOWN);
    SDL_SetWindowFullscreen(app.window, game_settings.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, game_settings.vsync ? "1" : "0");
    app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);

    create_screen_textures();
}


void destroy_game_window() {
    SDL_DestroyWindow(app.window);
    app.window = NULL;

    // This will also destroy all textures
    SDL_DestroyRenderer(app.renderer);
    app.renderer = NULL;
    app.shadow_texture = NULL;
    app.light_texture = NULL;
    app.blood_texture = NULL;
    app.blood_threshold_texture = NULL;
    app.blood_multiply_texture = NULL;
}


void resize_game_window() {
    int w;
    int h;
    SDL_GetWindowSize(app.window, &w, &h);
    if (game_settings.width != w || game_settings.height != h) {
        SDL_SetWindowSize(app.window, game_settings.width, game_settings.height);

        #ifdef __EMSCRIPTEN__
            emscripten_set_canvas_element_size("#canvas", game_settings.width, game_settings.height);
            // EM_ASM({
            //     var canvas = document.getElementById('canvas');
            //     canvas.style.width = $0 + "px";
            //     canvas.style.height = $1 + "px";
            // }, game_settings.width, game_settings.height);
        #endif
        
        SDL_DestroyTexture(app.shadow_texture);
        SDL_DestroyTexture(app.light_texture);
        SDL_DestroyTexture(app.blood_texture);
        SDL_DestroyTexture(app.blood_threshold_texture);
        SDL_DestroyTexture(app.blood_multiply_texture);

        create_screen_textures();
    }

    #ifndef __EMSCRIPTEN__
        SDL_SetWindowFullscreen(app.window, game_settings.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    #else
        if (game_settings.fullscreen) {
            emscripten_request_fullscreen("#canvas", EM_FALSE);
        } else {
            emscripten_exit_fullscreen();
        }
    #endif
}


void init() {
    setbuf(stdout, NULL);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_ShowCursor(SDL_DISABLE);
    
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);
    Mix_AllocateChannels(32);

    memset(app.controllers, 0, sizeof(app.controllers));

    int controllers = SDL_NumJoysticks();
    for (int i = 0; i < controllers; i++) {
        if (SDL_IsGameController(i)) {
            if (app.controllers[i] == NULL) {
                app.controllers[i] = SDL_GameControllerOpen(i);
            }
        }
    }
    for (int i = 0; i < 4; i++) {
        app.player_controllers[i] = CONTROLLER_NONE;
    }
    app.player_controllers[0] = CONTROLLER_MKB;

    create_game_window();

    app.fps = FpsCounter_create();

    app.quit = false;
    app.focus = true;
    app.time_step = 1.0f / 60.0f;
    app.delta = 0.0f;

    network_init();
}


void quit() {
    free(app.fps);
    destroy_game_window();

    network_shutdown();

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


void input_game(SDL_Event sdl_event) {
    switch (game_state) {
        case STATE_CREATE_LOBBY:
            if (!network_host_start(NET_DEFAULT_PORT)) {
                LOG_ERROR("Failed to start host");
                game_state = STATE_MENU;
            } else {
                rebuild_host_player_controllers();
                destroy_menu();
                create_host_lobby_menu();
                game_state = STATE_HOST_LOBBY;
            }
            break;
        case STATE_JOIN:
            if (!network_client_connect(network.host_ip, NET_DEFAULT_PORT)) {
                LOG_ERROR("Failed to connect to host");
                game_state = STATE_MENU;
            } else {
                destroy_menu();
                create_lobby_menu();
                game_state = STATE_CLIENT_LOBBY;
            }
            break;
        case STATE_GAME:
            if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.repeat == 0) {
                if (sdl_event.key.keysym.sym == SDLK_ESCAPE || sdl_event.key.keysym.sym == SDLK_p) {
                    if (game_data->testing) {
                        game_state = STATE_LOAD_EDITOR;
                    } else {
                        game_state = STATE_PAUSE;
                    }
                } else if (sdl_event.key.keysym.sym == SDLK_F1) {
                    if (game_settings.debug) {
                        debug_level = (debug_level + 1) % 4;
                    }
                }
            }

            if (game_settings.debug) {
                if (sdl_event.type == SDL_MOUSEWHEEL) {
                    CameraComponent* camera = CameraComponent_get(game_data->camera);
                    if (sdl_event.wheel.y > 0) {
                        camera->zoom_target = fminf(camera->zoom_target * 1.1f, 100.0f);
                    } else if (sdl_event.wheel.y < 0) {
                        camera->zoom_target = fmaxf(camera->zoom_target / 1.1f, 10.0f);
                    }
                }
            }
            break;
        case STATE_HOST:
            if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.repeat == 0) {
                if (sdl_event.key.keysym.sym == SDLK_ESCAPE || sdl_event.key.keysym.sym == SDLK_p) {
                    game_state = STATE_HOST_PAUSE;
                }
            }
            if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_TAB) {
                show_leaderboard = true;
            } else if (sdl_event.type == SDL_KEYUP && sdl_event.key.keysym.sym == SDLK_TAB) {
                show_leaderboard = false;
            }
            break;
        case STATE_HOST_END:
            break;
        case STATE_HOST_GAME_OVER:
        case STATE_CLIENT_GAME_OVER:
        case STATE_GAME_OVER:
            input_menu(game_data->menu_camera, sdl_event);
            break;
        case STATE_CLIENT:
            if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.repeat == 0) {
                if (sdl_event.key.keysym.sym == SDLK_ESCAPE || sdl_event.key.keysym.sym == SDLK_p) {
                    game_state = STATE_CLIENT_PAUSE;
                }
            }
            if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_TAB) {
                show_leaderboard = true;
            } else if (sdl_event.type == SDL_KEYUP && sdl_event.key.keysym.sym == SDLK_TAB) {
                show_leaderboard = false;
            }
            break;
        case STATE_PAUSE:
        case STATE_HOST_PAUSE:
        case STATE_CLIENT_PAUSE:
            if (sdl_event.type == SDL_KEYDOWN) {
                if (sdl_event.key.keysym.sym == SDLK_p || sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                    game_state = previous_state;
                }
            }
            input_menu(game_data->menu_camera, sdl_event);
            break;
        case STATE_EDITOR:
            input_editor(sdl_event);
            break;
        case STATE_MENU:
        case STATE_HOST_LOBBY:
        case STATE_CLIENT_LOBBY:
            input_menu(game_data->menu_camera, sdl_event);
            break;
        case STATE_INTRO:
            if (sdl_event.type == SDL_KEYDOWN) {
                if (sdl_event.key.keysym.sym == SDLK_p || sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                    game_state = STATE_START;
                }
            } else if (sdl_event.type == SDL_MOUSEBUTTONDOWN) {
                intro.panel++;
                if (intro.panel >= 6) {
                    intro.panel = 0;
                    intro.page++;
                    if (intro.page > 4) {
                        game_state = STATE_START;
                    }
                }
            }
            break;
    default:
        break;
    }

}


void input() {
    GameState state = game_state;

    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type) {
            case SDL_WINDOWEVENT:
                if (sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    app.focus = false;
                } else if (sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    app.focus = true;
                }
                break;
            case SDL_QUIT:
                app.quit = true;
                return;
            case SDL_JOYDEVICEADDED:
                if (app.controllers[sdl_event.jdevice.which] == NULL) {
                    app.controllers[sdl_event.jdevice.which] = SDL_GameControllerOpen(sdl_event.jdevice.which);
                    LOG_INFO("Joystick added: %d", sdl_event.jdevice.which)
                }
                break;
            case SDL_JOYDEVICEREMOVED:
                for (int i = 0; i < 8; i++) {
                    if (app.controllers[i] == NULL) continue;

                    if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(app.controllers[i])) == sdl_event.jdevice.which) {
                        SDL_GameControllerClose(app.controllers[i]);
                        app.controllers[i] = NULL;
                        LOG_INFO("Joystick removed: %d", sdl_event.jdevice.which)
                    }
                }
                break;
            default:
                input_game(sdl_event);
                break;
        }
    }

    if (state != game_state) {
        previous_state = state;
    }
}


void update(float time_step) {
    GameState state = game_state;

    switch (game_state) {
        case STATE_MENU:
            intro.page = 1;
            intro.panel = 0;
            update_menu();
            break;
        case STATE_HOST_LOBBY:
            // Host: accept incoming client connections while in lobby
            network_host_accept_clients();
            rebuild_host_player_controllers();
            if (lobby_info_broadcast_timer <= 0.0f) {
                broadcast_lobby_info();
                lobby_info_broadcast_timer = 0.25f;
            } else {
                lobby_info_broadcast_timer = fmaxf(lobby_info_broadcast_timer - time_step, 0.0f);
            }
            update_menu();
            break;
        case STATE_CLIENT_LOBBY:
            ;
            // Client: check for JOIN_ACK or START_GAME
            struct sockaddr_in from;
            int received;
            while ((received = network_receive(network.recv_buf, NET_MAX_PACKET_SIZE, &from)) > 0) {
                if (received < (int)sizeof(PacketHeader)) continue;
                PacketHeader* hdr = (PacketHeader*)network.recv_buf;
                if (hdr->type == PACKET_JOIN_ACK && received >= (int)sizeof(JoinAckPacket)) {
                    JoinAckPacket* ack = (JoinAckPacket*)network.recv_buf;
                    network.local_player_slot = ack->player_slot;
                    LOG_INFO("Joined as player %d", network.local_player_slot);
                } else if (hdr->type == PACKET_LOBBY_INFO && received >= (int)sizeof(LobbyInfoPacket)) {
                    cache_lobby_info((LobbyInfoPacket*)network.recv_buf);
                } else if (hdr->type == PACKET_START_GAME && received >= (int)sizeof(StartGamePacket)) {
                    StartGamePacket* start = (StartGamePacket*)network.recv_buf;
                    strncpy(game_data->map_name, start->map_name, sizeof(game_data->map_name) - 1);
                    game_data->game_mode = (GameMode)start->game_mode;
                    // Set up controllers: only our slot is local, mark others as active
                    for (int i = 0; i < 4; i++) {
                        app.player_controllers[i] = CONTROLLER_NONE;
                    }
                    for (int i = 0; i < (int)start->num_players; i++) {
                        app.player_controllers[i] = CONTROLLER_MKB;  // all active
                    }
                    network.game_started = true;
                    lobby_info_broadcast_timer = 0.0f;
                    game_state = STATE_CLIENT_START;
                    break;
                }
            }
            update_menu();
            break;
        case STATE_START:
            start_game(game_data->map_name, false);
            create_pause_menu();
            game_state = STATE_GAME;
            break;
        case STATE_HOST_START:
            rebuild_host_player_controllers();
            start_game(game_data->map_name, false);
            set_player_names(cached_lobby_info.player_names, cached_lobby_info.num_players);
            reset_host_client_timeouts();

            // Count total players (host + connected clients)
            int num_players = 1;
            for (int i = 0; i < NET_MAX_CLIENTS; i++) {
                if (network.clients[i].connected) num_players++;
            }
            // Broadcast START_GAME to clients
            StartGamePacket start_pkt;
            memset(&start_pkt, 0, sizeof(start_pkt));
            start_pkt.header.type = PACKET_START_GAME;
            start_pkt.header.tick = network.tick;
            start_pkt.header.size = sizeof(StartGamePacket);
            strncpy(start_pkt.map_name, game_data->map_name, sizeof(start_pkt.map_name) - 1);
            start_pkt.game_mode = (uint8_t)game_data->game_mode;
            start_pkt.num_players = (uint8_t)num_players;
            network_broadcast(&start_pkt, sizeof(start_pkt));
            network.game_started = true;
            lobby_info_broadcast_timer = 0.0f;

            // Initialize net_entity_seen and ID mappings for both host and client
            memset(net_entity_seen, 0, sizeof(net_entity_seen));

            create_host_pause_menu();
            game_state = STATE_HOST;
            break;
        case STATE_CLIENT_START:
            start_game(game_data->map_name, false);
            set_player_names(cached_lobby_info.player_names, cached_lobby_info.num_players);

            // Initialize net_entity_seen and ID mappings for both host and client
            memset(net_entity_seen, 0, sizeof(net_entity_seen));

            create_client_pause_menu();
            game_state = STATE_CLIENT;
            break;
        case STATE_HOST_END:
            return_to_lobby();
            break;
        case STATE_HOST_GAME_OVER:
            update_game_over(time_step);
            break;
        case STATE_CLIENT_GAME_OVER:
            if (client_receive_packets()) {
                break;
            }

            update_game_over(time_step);
            break;
        case STATE_GAME_OVER:
            input_players(game_data->camera);
            update_game_over(time_step);
            break;
        case STATE_END:
            end_game();
            clear_all_sounds();
            if (network.mode != NET_MODE_NONE) {
                network_shutdown();
                network_init();
            }
            network.game_started = false;
            game_state = STATE_MENU;
            break;
        case STATE_RESET:
            end_game();
            clear_all_sounds();
            game_state = STATE_START;
            break;
        case STATE_GAME:
            input_players(game_data->camera);
            update_game(time_step);
            update_game_mode(time_step);
            break;
        case STATE_HOST:
        {
            // Host: receive remote inputs FIRST so all controllers are fresh,
            // then read local input + run state machines, then simulate.

            float current_time = SDL_GetTicks() / 1000.0f;

            // 1. Receive and apply remote inputs
            struct sockaddr_in from;
            int received;
            while ((received = network_receive(network.recv_buf, NET_MAX_PACKET_SIZE, &from)) > 0) {
                if (received < (int)sizeof(PacketHeader)) continue;
                PacketHeader* hdr = (PacketHeader*)network.recv_buf;
                if (hdr->type == PACKET_INPUT && received >= (int)sizeof(InputPacket)) {
                    InputPacket* input_pkt = (InputPacket*)network.recv_buf;
                    // Update last_recv_time for the sending client
                    for (int ci = 0; ci < NET_MAX_CLIENTS; ci++) {
                        if (network.clients[ci].connected &&
                            from.sin_addr.s_addr == network.clients[ci].addr.sin_addr.s_addr &&
                            from.sin_port == network.clients[ci].addr.sin_port) {
                            network.clients[ci].last_recv_time = current_time;
                            break;
                        }
                    }
                    // Find the player entity for this slot
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

            // 1b. Check for client timeouts
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

            // 2. Read local input + run state machines for all players
            // (remote players now have fresh controller data from step 1)
            input_players(game_data->camera);

            // 3. Simulate
            update_game(time_step);
            update_game_mode(time_step);

            if (game_state == STATE_HOST_LOBBY) {
                return_to_lobby();
                break;
            }

            // 4. Build and broadcast snapshot
            int snap_size = netgame_build_snapshot(network.send_buf, NET_MAX_PACKET_SIZE, network.tick);
            network_broadcast(network.send_buf, snap_size);
            network.tick++;
            break;
        }
        case STATE_CLIENT:
        {
            // Client: only read local controller (no state machine -- host is authoritative),
            // send input to host, receive and apply snapshots.

            // 1. Update local controller and send input to host.
            netgame_client_send_input(false);

            // 3. Save current positions as "previous" BEFORE applying new snapshot,
            //    so the interpolation system has two distinct states to lerp between.
            update_coordinates();

            // 4. Receive and apply snapshots (overwrites current positions)
            if (client_receive_packets()) {
                break;
            }

            // 5. Rebuild collision grid so lights raycast against correct positions
            ColliderGrid_clear(game_data->grid);
            init_grid();

            // 6. Update camera to follow players
            update_camera(game_data->camera, time_step, true);

            // 7. Update light brightness ramp and flicker animation
            update_lights(time_step);

            // 8. Update particle simulation (position, lifetime, emission)
            update_particles(game_data->camera, time_step);

            // Snap camera's previous state to current so interpolation doesn't
            // fight with the exponential smoothing. The smoothing already provides
            // continuity; interpolating on top of it causes desync with entity positions.
            CoordinateComponent* cam_coord = CoordinateComponent_get(game_data->camera);
            cam_coord->previous.position = cam_coord->position;

            network.tick++;
            break;
        }
        case STATE_HOST_PAUSE:
            if (game_state == STATE_HOST_LOBBY) {
                return_to_lobby();
                break;
            }
            update_menu();
            break;
        case STATE_CLIENT_PAUSE:
        {
            netgame_client_send_input(true);

            // Preserve interpolation while paused before applying fresh snapshots.
            update_coordinates();

            if (client_receive_packets()) {
                break;
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
            update_menu();
            break;
        }
        case STATE_PAUSE:
            update_menu();
            break;
        case STATE_SAVE:
            save_state(game_data->map_name);
            game_state = previous_state;
            break;
        case STATE_LOAD:
            end_game();
            clear_all_sounds();
            start_game(game_data->map_name, true);
            create_pause_menu();
            game_state = STATE_GAME;
            break;
        case STATE_APPLY:
            resize_game();
            resize_game_window();
            save_settings();
            game_state = STATE_MENU;
            break;
        case STATE_CREATE:
            destroy_menu();
            create_editor_menu();
            game_state = STATE_EDITOR;
            break;
        case STATE_LOAD_EDITOR:
            game_data->testing = false;
            end_game();
            CoordinateComponent_get(game_data->camera)->position = game_data->start_position;
            create_editor_menu();
            load_map(game_data->map_name);
            init_grid();
            game_state = STATE_EDITOR;
            break;
        case STATE_EDITOR:
            update_editor(time_step);
            break;
        case STATE_QUIT:
            app.quit = true;
            return;
        case STATE_INTRO:
            ;
            Vector2f target = mult(intro.scale * 9.0f, vec(0.5f - intro.panel % 2, intro.panel / 2 - 1));
            intro.position = sum(intro.position, mult(0.1f, diff(target, intro.position)));
            break;
    }

    if (state != game_state) {
        previous_state = state;
    }

    float time = SDL_GetTicks() / 1000.0f;
    title_scale = 2.5f + 0.1f * sinf(2.0f * time);
}


void draw() {
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app.renderer);

    switch (game_state) {
        case STATE_MENU:
            draw_sprite(game_data->menu_camera, get_texture_index("menu"), 0, 0, 0, zeros(), 0.0f, mult(3.5f, ones()), 1.0f);
            draw_sprite(game_data->menu_camera, get_texture_index("title"), 0, 0, 0, vec(0.0f, 9.0f), 0.0f, vec(title_scale, title_scale), 1.0f);
            draw_menu();

            String buffer;
            snprintf(buffer, STRING_SIZE, "v%s", version);
            Color color = get_color(1.0f, 1.0f, 0.0f, 0.5f);
            draw_text(game_data->menu_camera, vec(24.5f, -13.5f), buffer, 20, color);
            break;
        case STATE_CLIENT_LOBBY:
            draw_menu();
            if (cached_lobby_info.valid) {
                draw_text(game_data->menu_camera, vec(0.0f, 12.0f), network.host_ip, 20, COLOR_WHITE);

                String buffer;
                snprintf(buffer, STRING_SIZE, "Mode: %s", game_mode_to_string(cached_lobby_info.game_mode));
                draw_text(game_data->menu_camera, vec(0.0f, 5.0f), buffer, 20, COLOR_WHITE);

                snprintf(buffer, STRING_SIZE, "Map: %s", cached_lobby_info.map_name);
                draw_text(game_data->menu_camera, vec(0.0f, 7.0f), buffer, 20, COLOR_WHITE);

                snprintf(buffer, STRING_SIZE, "Point Limit: %d", cached_lobby_info.point_limit);
                draw_text(game_data->menu_camera, vec(0.0f, 3.0f), buffer, 20, COLOR_WHITE);

                for (int i = 0; i < cached_lobby_info.num_players && i < NET_MAX_CLIENTS + 1; i++) {
                    draw_text(game_data->menu_camera, vec(0.0f, -1.0f - i * 2.0f), cached_lobby_info.player_names[i], 20, COLOR_WHITE);
                }
            }
            break;
        case STATE_HOST_LOBBY:
            draw_menu();
            draw_text(game_data->menu_camera, vec(0.0f, 12.0f), network.own_ip, 20, COLOR_WHITE);
            for (int i = 0; i < NET_MAX_CLIENTS; i++) {
                if (network.clients[i].connected) {
                    String buffer;
                    snprintf(buffer, STRING_SIZE, "%s - %s", network.clients[i].player_name,  network.clients[i].ip);
                    draw_text(game_data->menu_camera, vec(0.0f, -5.0f + i * 2.0f), buffer, 20, COLOR_WHITE);
                }
            }
            break;
        case STATE_HOST_END:
        case STATE_HOST_GAME_OVER:
        case STATE_CLIENT_GAME_OVER:
        case STATE_GAME_OVER:
            draw_game_over();
            break;
        case STATE_START:
        case STATE_END:
        case STATE_RESET:
        case STATE_LOAD_EDITOR:
        case STATE_HOST_START:
        case STATE_CLIENT_START:
            draw_text(game_data->menu_camera, zeros(), "LOADING", 20, COLOR_WHITE);
            break;
        case STATE_HOST:
        case STATE_CLIENT:
        case STATE_GAME:
            draw_game();
            draw_hud(game_data->camera);
            draw_game_mode();
            if (show_leaderboard) {
                draw_leaderboard(game_data->menu_camera);
            }
            break;
        case STATE_HOST_PAUSE:
        case STATE_CLIENT_PAUSE:
        case STATE_PAUSE:
            app.delta = 0.0f;
            draw_game();
            draw_overlay(game_data->camera, 0.4f);
            draw_menu();
            break;
        case STATE_EDITOR:
            draw_editor();
            draw_hud(game_data->camera);
            break;
        case STATE_INTRO:
            ;
            String filename;
            snprintf(filename, STRING_SIZE, "intro_%d", intro.page);
            draw_sprite(game_data->menu_camera, get_texture_index(filename), 0, 0, 0, intro.position, 0.0f, mult(intro.scale, ones()), 1.0f);

            Vector2f pos = get_mouse_position(game_data->menu_camera);
            draw_circle(game_data->menu_camera, pos, 0.1f, COLOR_WHITE);
            break;
        default:
            break;
    }

    if (debug_level) {
        draw_debug(debug_level);
    }

    FPSCounter_draw(app.fps);

    SDL_RenderPresent(app.renderer);
}


void play_audio() {
    static bool music_playing = false;
    static float music_fade = 0.0f;
    static int current_music = -1;

    switch(game_state) {
        case STATE_MENU:
            game_data->music = 0;
            break;
        case STATE_HOST:
        case STATE_CLIENT:
        case STATE_GAME:
        case STATE_INTRO:
        case STATE_HOST_PAUSE:
        case STATE_CLIENT_PAUSE:
        case STATE_PAUSE:
            game_data->music = 1;
            break;
        default:
            game_data->music = -1;
            break;
    }

    if (game_data->music != current_music) {
        music_fade = fmaxf(0.0f, music_fade - 0.01f);

        if (music_fade == 0.0f) {
            current_music = game_data->music;

            Mix_HaltMusic();
            if (game_data->music != -1) {
                Mix_PlayMusic(resources.music[game_data->music], -1);
            }
        }
    } else {
        music_fade = fminf(1.0f, music_fade + 0.01f);
    }

    play_sounds(game_data->camera);

    Mix_VolumeMusic(0.5f * game_settings.music * music_fade);

    if (!music_playing) {
        Mix_VolumeMusic(0);
        Mix_PlayMusic(resources.music[0], -1);
        music_playing = true;
    }
}
