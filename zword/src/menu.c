#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "menu.h"
#include "game.h"
#include "component.h"
#include "settings.h"
#include "widget.h"
#include "light.h"
#include "input.h"
#include "app.h"
#include "benchmark.h"
#include "netgame.h"
#include "network.h"
#include "serialize.h"
#include "util.h"


static String RESOLUTIONS[] = {"1280x720", "1360x768", "1600x900", "1920x1080", "2560x1440", "3840x2160"};
int RESOLUTION_ID = -1;
int fullscreen_id = -1;
static int map_name_textbox = -1;
static int window_play = -1;
static int window_new_map = -1;
static int window_editor = -1;
static int window_settings = -1;
static int window_controls = -1;
static int window_credits = -1;
static int window_keyboard_controls = -1;
static int window_xbox_controls = -1;
static int button_benchmark = -1;

static Entity ip_input_lan = NULL_ENTITY;
static Entity window_lan = NULL_ENTITY;
static Entity window_server_browser = NULL_ENTITY;
static Entity lan_map_dropdown = NULL_ENTITY;
static Entity lan_mode_dropdown = NULL_ENTITY;
static Entity client_lobby_mode_label = NULL_ENTITY;
static Entity client_lobby_map_label = NULL_ENTITY;
static Entity client_lobby_friendly_fire_label = NULL_ENTITY;
static Entity client_lobby_point_limit_label = NULL_ENTITY;
static Entity lobby_player_panel = NULL_ENTITY;
static Entity lobby_player_container = NULL_ENTITY;
static Entity lobby_player_header = NULL_ENTITY;
static Entity server_list = NULL_ENTITY;

static float server_browser_refresh_time = 0.0f;


void reset_ids() {
    RESOLUTION_ID = -1;
    fullscreen_id = -1;
    window_play = -1;
    window_new_map = -1;
    window_editor = -1;
    window_settings = -1;
    window_controls = -1;
    window_credits = -1;
    window_keyboard_controls = -1;
    window_xbox_controls = -1;
    button_benchmark = -1;

    ip_input_lan = NULL_ENTITY;
    window_lan = NULL_ENTITY;
    lan_map_dropdown = NULL_ENTITY;
    lan_mode_dropdown = NULL_ENTITY;
    client_lobby_mode_label = NULL_ENTITY;
    client_lobby_map_label = NULL_ENTITY;
    client_lobby_friendly_fire_label = NULL_ENTITY;
    client_lobby_point_limit_label = NULL_ENTITY;
    lobby_player_panel = NULL_ENTITY;
    lobby_player_container = NULL_ENTITY;
    lobby_player_header = NULL_ENTITY;
    window_server_browser = NULL_ENTITY;
    server_list = NULL_ENTITY;
    server_browser_refresh_time = -1.0f;
}


static void refresh_server_list(Entity entity) {
    UNUSED(entity);
    netgame_clear_discovered_servers();
    if (!network_send_discover(NET_DEFAULT_PORT)) {
        LOG_WARNING("Failed to send LAN discover packet");
    }
    server_browser_refresh_time = 0.5f;
}


static void recreate_lobby_player_container(void) {
    if (lobby_player_container == NULL_ENTITY) {
        return;
    }

    remove_parent(lobby_player_container);
    destroy_entity_recursive(lobby_player_container);

    lobby_player_container = create_container(zeros(), 1, 5);
    WidgetComponent_get(lobby_player_container)->enabled = false;
    add_child(lobby_player_panel, lobby_player_container);
}


static void update_lobby_player_panel(void) {
    if (lobby_player_panel == NULL_ENTITY ||
        lobby_player_container == NULL_ENTITY ||
        lobby_player_header == NULL_ENTITY) {
        return;
    }

    recreate_lobby_player_container();

    WidgetComponent* header = WidgetComponent_get(lobby_player_header);
    if (network.mode == NET_MODE_HOST) {
        strcpy(header->string, network.own_ip);

        String buffer;
        snprintf(buffer, STRING_SIZE, "%s - %s", game_settings.player_name, network.own_ip);
        add_widget_to_container(lobby_player_container, create_label(buffer, zeros()));

        for (int i = 0; i < NET_MAX_CLIENTS; i++) {
            if (!network.clients[i].connected) {
                continue;
            }

            snprintf(buffer, STRING_SIZE, "%s - %s", network.clients[i].player_name, network.clients[i].ip);
            add_widget_to_container(lobby_player_container, create_label(buffer, zeros()));
        }
    } else if (network.mode == NET_MODE_CLIENT) {
        strcpy(header->string, network.host_ip);

        if (!cached_lobby_info.valid) {
            return;
        }

        for (int i = 0; i < cached_lobby_info.num_players; i++) {
            add_widget_to_container(lobby_player_container, create_label(cached_lobby_info.player_names[i], zeros()));
        }
    }

    if (CoordinateComponent_get(lobby_player_container)->children->size > 5) {
        add_scrollbar_to_container(lobby_player_container);
    }
}


static void update_client_lobby_menu_info(void) {
    if (client_lobby_mode_label == NULL_ENTITY ||
        client_lobby_map_label == NULL_ENTITY ||
        client_lobby_point_limit_label == NULL_ENTITY) {
        return;
    }

    WidgetComponent* mode = WidgetComponent_get(client_lobby_mode_label);
    WidgetComponent* map = WidgetComponent_get(client_lobby_map_label);
    WidgetComponent* friendly_fire = WidgetComponent_get(client_lobby_friendly_fire_label);
    WidgetComponent* point_limit = WidgetComponent_get(client_lobby_point_limit_label);

    if (!cached_lobby_info.valid) {
        strcpy(mode->string, "Waiting...");
        strcpy(map->string, "Waiting...");
        strcpy(friendly_fire->string, "Waiting...");
        strcpy(point_limit->string, "Waiting...");
        return;
    }

    strcpy(mode->string, GAME_MODES[cached_lobby_info.game_mode]);
    strcpy(map->string, cached_lobby_info.map_name);
    String strings[] = { "OFF", "ON" };
    strcpy(friendly_fire->string, strings[cached_lobby_info.friendly_fire]);
    snprintf(point_limit->string, STRING_SIZE, "%d", cached_lobby_info.point_limit);
}


static void update_lobby_menu(void) {
    if (game_state == STATE_HOST_LOBBY || game_state == STATE_CLIENT_LOBBY) {
        if (game_state == STATE_CLIENT_LOBBY) {
            update_client_lobby_menu_info();
        }

        update_lobby_player_panel();
    }
}


static void create_lobby_player_panel() {
    lobby_player_panel = create_menu_entity();
    CoordinateComponent_add(lobby_player_panel, vec(0.0f, -1.0f), 0.0f);
    ColliderComponent_add_rectangle(lobby_player_panel, BUTTON_WIDTH, 6 * BUTTON_HEIGHT, GROUP_WALLS)->enabled = false;
    WidgetComponent_add(lobby_player_panel, "", WIDGET_LABEL)->enabled = false;

    lobby_player_header = create_label("", vec(0.0f, 14.0f));
    add_child(lobby_player_panel, lobby_player_header);

    lobby_player_container = create_container(zeros(), 1, 5);
    WidgetComponent_get(lobby_player_container)->enabled = false;
    add_child(lobby_player_panel, lobby_player_container);
}


void change_state_end(int entity) {
    UNUSED(entity);
    game_state = STATE_END;
    reset_ids();
}


void change_state_lobby(int entity) {
    UNUSED(entity);
    if (network.mode == NET_MODE_HOST) {
        game_state = STATE_HOST_END;
    } else if (network.mode == NET_MODE_CLIENT) {
        game_state = STATE_CLIENT_GAME_OVER;
    } else {
        game_state = STATE_MENU;
    }
    reset_ids();
}


void change_state_create(int entity) {
    UNUSED(entity);
    strcpy(game_data->map_name, WidgetComponent_get(map_name_textbox)->string);
    game_state = STATE_CREATE;
    reset_ids();
}


void change_state_start(int entity) {
    WidgetComponent* widget = WidgetComponent_get(entity);
    strcpy(game_data->map_name, widget->string);
    game_state = STATE_START;
    reset_ids();
}


void change_state_reset(int entity) {
    UNUSED(entity);
    game_state = STATE_RESET;
    reset_ids();
}


void change_state_game(int entity) {
    UNUSED(entity);
    game_state = STATE_GAME;
    reset_ids();
}


void change_state_create_lobby(Entity entity) {
    UNUSED(entity);
    save_settings();
    game_state = STATE_CREATE_LOBBY;
    reset_ids();
}


void change_state_join(Entity entity) {
    UNUSED(entity);
    strcpy(game_settings.last_ip, network.host_ip);
    save_settings();
    game_state = STATE_JOIN;
    reset_ids();
}


void change_state_host_game(int entity) {
    UNUSED(entity);
    game_state = STATE_HOST;
    reset_ids();
}


void change_state_client_game(int entity) {
    UNUSED(entity);
    game_state = STATE_CLIENT;
    reset_ids();
}


void change_state_host_start(int entity) {
    UNUSED(entity);
    game_state = STATE_HOST_START;
    reset_ids();
}


void change_state_save(int entity) {
    UNUSED(entity);
    game_state = STATE_SAVE;
}


void change_state_load(int entity) {
    UNUSED(entity);
    game_state = STATE_LOAD;
}


void change_state_load_editor(int entity) {
    WidgetComponent* widget = WidgetComponent_get(entity);
    strcpy(game_data->map_name, widget->string);
    game_state = STATE_LOAD_EDITOR;
    reset_ids();
}


void change_state_quit(int entity) {
    UNUSED(entity);
    game_state = STATE_QUIT;
}


void apply(int entity) {
    UNUSED(entity);

    WidgetComponent* widget = WidgetComponent_get(RESOLUTION_ID);
    ButtonText text;
    strcpy(text, widget->strings[widget->value]);
    char* width = strtok(text, "x");
    char* height = strtok(NULL, "x");
    game_settings.width = strtol(width, NULL, 10);
    game_settings.height = strtol(height, NULL, 10);

    widget = WidgetComponent_get(fullscreen_id);
    game_settings.fullscreen = widget->value;

    game_state = STATE_APPLY;
}


void toggle_survival(int entity) {
    UNUSED(entity);
    if (window_play != -1) {
        destroy_entity_recursive(window_play);
        window_play = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_play = create_window(pos, "PLAY", 1, toggle_survival);

    int container = create_container(vec(0.0f, -2.0f * BUTTON_HEIGHT), 1, 3);
    add_child(window_play, container);

    if (game_settings.debug) {
        add_files_to_container(container, "maps", change_state_start);
        add_scrollbar_to_container(container);
    } else {
        add_button_to_container(container, "Survival", change_state_start);
        add_button_to_container(container, "Swampland", change_state_start);
    }
}


void join_discovered_server(Entity entity) {
    WidgetComponent* widget = WidgetComponent_get(entity);
    int server_index = widget->value;
    if (server_index < 0 || server_index >= discovered_servers_count) {
        return;
    }
    strcpy(network.host_ip, discovered_servers[server_index].host_ip);
    change_state_join(entity);
}


void update_server_list(float time_step) {
    if (window_server_browser == NULL_ENTITY) {
        return;
    }

    if (server_browser_refresh_time < 0.0f) {
        return;
    }

    struct sockaddr_in from;
    int received;
    while ((received = network_receive(network.recv_buf, NET_MAX_PACKET_SIZE, &from)) > 0) {
        if (received != (int)sizeof(DiscoverResponsePacket)) {
            continue;
        }

        PacketHeader* header = (PacketHeader*)network.recv_buf;
        if (header->type != PACKET_DISCOVER_RESPONSE) {
            continue;
        }

        DiscoverResponsePacket* response = (DiscoverResponsePacket*)network.recv_buf;
        netgame_store_discovered_server(response, &from);
    }

    server_browser_refresh_time = fmaxf(server_browser_refresh_time - time_step, 0.0f);
    if (server_browser_refresh_time > 0.0f) {
        return;
    }

    clear_container(server_list);

    for (int i = 0; i < discovered_servers_count; i++) {
        DiscoveredServer* server = &discovered_servers[i];
        if (!server->valid) {
            continue;
        }

        String buffer;
        snprintf(buffer, STRING_SIZE, "%s's lobby (%d/%d)",
            server->host_name,
            server->num_players,
            server->max_players
        );

        Entity label = create_label(buffer, zeros());
        Entity button = create_button("JOIN", vec(150.0f, 0.0f), join_discovered_server);
        add_row_to_container(server_list, label, button);
        WidgetComponent_get(button)->value = i;
    }

    server_browser_refresh_time = -1.0f;
}


void toggle_server_browser(Entity entity) {
    UNUSED(entity);
    if (window_server_browser != NULL_ENTITY) {
        destroy_entity_recursive(window_server_browser);
        window_server_browser = NULL_ENTITY;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_server_browser = create_window(pos, "SERVER BROWSER", 2, toggle_server_browser);

    server_list = create_container(vec(0.0f, -3.0f * BUTTON_HEIGHT), 2, 5);
    add_child(window_server_browser, server_list);

    Entity refresh_button = create_button("REFRESH", vec(0.0f, -5.0f * BUTTON_HEIGHT), refresh_server_list);
    add_child(window_server_browser, refresh_button);

    refresh_server_list(refresh_button);
}


void update_player_name(Entity entity, int value) {
    UNUSED(value);
    WidgetComponent* widget = WidgetComponent_get(entity);
    strcpy(game_settings.player_name, widget->string);
}


void update_host_ip(Entity entity, int value) {
    UNUSED(value);
    WidgetComponent* widget = WidgetComponent_get(entity);
    strcpy(network.host_ip, widget->string);
}


void toggle_lan(Entity entity) {
    UNUSED(entity);
    if (window_lan != NULL_ENTITY) {
        destroy_entity_recursive(window_lan);
        window_lan = NULL_ENTITY;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_lan = create_window(pos, "LAN", 2, toggle_lan);

    Entity container = create_container(vec(0.0f, -2.0f * BUTTON_HEIGHT), 2, 3);
    add_child(window_lan, container);

    Entity name_label = create_label("NAME", zeros());
    Entity name_input = create_textbox(zeros(), 1, game_settings.player_name, update_player_name);
    add_row_to_container(container, name_label, name_input);

    ip_input_lan = create_textbox(zeros(), 1, game_settings.last_ip, update_host_ip);
    Entity join_button = create_button("JOIN", zeros(), change_state_join);
    add_row_to_container(container, ip_input_lan, join_button);

    Entity browse_button = create_button("BROWSE", zeros(), toggle_server_browser);
    Entity host_button = create_button("HOST", zeros(), change_state_create_lobby);
    add_row_to_container(container, browse_button, host_button);
}


void toggle_new_map(int entity) {
    UNUSED(entity);
    if (window_new_map != -1) {
        destroy_entity_recursive(window_new_map);
        window_new_map = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_new_map = create_window(pos, "NEW MAP", 2, toggle_new_map);

    int container = create_container(vec(0.0f, -1.5f * BUTTON_HEIGHT), 2, 2);
    add_child(window_new_map, container);

    int label = create_label("NAME", zeros());
    map_name_textbox = create_textbox(zeros(), 1, "", NULL);
    add_row_to_container(container, label, map_name_textbox);
    add_button_to_container(container, "CREATE", change_state_create);
}


void toggle_editor(int entity) {
    UNUSED(entity);
    if (window_editor != -1) {
        destroy_entity_recursive(window_editor);
        window_editor = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_editor = create_window(pos, "EDITOR", 1, toggle_editor);

    int container = create_container(vec(0.0f, -3 * BUTTON_HEIGHT), 1, 5);
    add_child(window_editor, container);

    add_button_to_container(container, "NEW MAP", toggle_new_map);
    add_files_to_container(container, "maps", change_state_load_editor);
}


int get_resolution_index() {
    char resolution[128];
    snprintf(resolution, 128, "%dx%d", game_settings.width, game_settings.height);
    int size = sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(RESOLUTIONS[i], resolution) == 0) {
            return i;
        }
    }
    return 0;
}


void set_volume(int entity, int value) {
    UNUSED(entity);
    game_settings.volume = value;
}


void set_music(int entity, int value) {
    UNUSED(entity);
    game_settings.music = value;
}


void toggle_settings(int entity) {
    UNUSED(entity);
    if (window_settings != -1) {
        destroy_entity_recursive(window_settings);
        window_settings = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_settings = create_window(pos, "SETTINGS", 2, toggle_settings);

    int container = create_container(vec(0.0f, -3.0f * BUTTON_HEIGHT), 2, 5);
    add_child(window_settings, container);

    int label = -1;
    label = create_label("Resolution", zeros());
    RESOLUTION_ID = create_dropdown(zeros(), RESOLUTIONS, sizeof(RESOLUTIONS) / sizeof(RESOLUTIONS[0]));
    WidgetComponent_get(RESOLUTION_ID)->value = get_resolution_index();
    add_row_to_container(container, label, RESOLUTION_ID);

    label = create_label("Fullscreen", zeros());
    fullscreen_id = create_checkbox(zeros(), game_settings.fullscreen, NULL);
    add_row_to_container(container, label, fullscreen_id);

    label = create_label("Sound", zeros());
    int slider = create_slider(zeros(), 0, 100, game_settings.volume, set_volume);
    add_row_to_container(container, label, slider);

    label = create_label("Music", zeros());
    slider = create_slider(zeros(), 0, 100, game_settings.music, set_music);
    add_row_to_container(container, label, slider);

    add_button_to_container(container, "Apply", apply);
}


void toggle_keyboard_controls(int entity) {
    UNUSED(entity);
    if (window_keyboard_controls != -1) {
        destroy_entity_recursive(window_keyboard_controls);
        window_keyboard_controls = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_keyboard_controls = create_window(pos, "KEYBOARD CONTROLS", 2, toggle_keyboard_controls);

    int container = create_container(vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(window_keyboard_controls, container);

    for (int i = 0; i < ACTIONS_SIZE; i++) {
        int label = create_label(ACTIONS[i], zeros());
        int button = create_button(keybind_to_string(game_settings.keybinds[i]), zeros(), NULL);
        add_row_to_container(container, label, button);
    }
    add_scrollbar_to_container(container);
}


void toggle_xbox_controls(int entity) {
    UNUSED(entity);
    if (window_xbox_controls != -1) {
        destroy_entity_recursive(window_xbox_controls);
        window_xbox_controls = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_xbox_controls = create_window(pos, "CONTROLS", 2, toggle_xbox_controls);

    int container = create_container(vec(0.0f, -3 * BUTTON_HEIGHT), 2, 5);
    add_child(window_xbox_controls, container);

    for (int i = 0; i < ACTIONS_SIZE; i++) {
        int label = create_label(ACTIONS[i], zeros());
        int button = create_button(ACTION_BUTTONS_XBOX[i], zeros(), NULL);
        add_row_to_container(container, label, button);
    }
    add_scrollbar_to_container(container);
}


void set_controller(int entity, int value) {
    int parent = get_parent(entity);

    int player = 0;
    int i = 0;
    ListNode* node;
    FOREACH(node, get_children(parent)) {
        if (entity == node->value) {
            player = i / 2;
        } else {
            WidgetComponent* widget = WidgetComponent_get(node->value);
            // Don't allow multiple players to use the same controller
            if (widget->value == value) {
                widget->value = 0;
            }
        }
        i++;
    }
    app.player_controllers[player] = value - 2;
    LOG_DEBUG("Player %d controller: %d", player, app.player_controllers[player]);
}


void toggle_controls(int entity) {
    UNUSED(entity);
    if (window_controls != -1) {
        destroy_entity_recursive(window_controls);
        window_controls = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_controls = create_window(pos, "CONTROLS", 2, toggle_controls);

    int container = create_container(vec(0.0f, -3.0f * BUTTON_HEIGHT), 2, 5);
    add_child(window_controls, container);

    static String CONTROLLERS[9] = {"None", "Keyboard", "", "", "", "", "", "", ""};
    
    for (int i = 0; i < 8; i++) {
        if (app.controllers[i] == NULL) {
            continue;
        }
        strcpy(CONTROLLERS[i + 2], SDL_GameControllerName(app.controllers[i]));
    }

    for (int i = 0; i < 4; i++) {
        char buffer[128];
        snprintf(buffer, 128, "Player %d", i + 1);
        int left = create_label(buffer, zeros());
        int right = create_dropdown(zeros(), CONTROLLERS, SDL_NumJoysticks() + 2);
        WidgetComponent* widget = WidgetComponent_get(right);
        widget->on_change = set_controller;
        widget->value = app.player_controllers[i] + 2;
        add_row_to_container(container, left, right);
    }

    int left = create_button("KEYBOARD", zeros(), toggle_keyboard_controls);
    int right = create_button("XBOX", zeros(), toggle_xbox_controls);
    add_row_to_container(container, left, right);
}


void toggle_credits(int entity) {
    UNUSED(entity);
    if (window_credits != -1) {
        destroy_entity_recursive(window_credits);
        window_credits = -1;
        return;
    }

    Vector2f pos = sum(vec(0.0f, 2 * BUTTON_HEIGHT), mult(BUTTON_HEIGHT, rand_vector()));
    window_credits = create_window(pos, "CREDITS", 2, toggle_credits);

    int container = create_container(vec(0.0f, -3.5f * BUTTON_HEIGHT), 2, 6);
    add_child(window_credits, container);

    add_row_to_container(container, create_label("Programming, art, music", zeros()), create_label("Panu Keskinen", zeros()));
    #ifdef __EMSCRIPTEN__
        add_row_to_container(container, create_label("Made with", zeros()), create_label("C, SDL2, cJSON, Emscripten", zeros()));
    #else 
        add_row_to_container(container, create_label("Made with", zeros()), create_label("C, SDL2, cJSON", zeros()));
    #endif
    add_row_to_container(container, create_label("Software used", zeros()), create_label("Visual Studio Code", zeros()));
    add_row_to_container(container, create_label("", zeros()), create_label("Gimp", zeros()));
    add_row_to_container(container, create_label("", zeros()), create_label("Ableton Live Lite", zeros()));
    add_row_to_container(container, create_label("", zeros()), create_label("Audacity", zeros()));
}


void update_benchmark(int entity) {
    UNUSED(entity);
    float fps = run_benchmark();

    if (button_benchmark != -1) {
        WidgetComponent* widget = WidgetComponent_get(button_benchmark);
        char buffer[128];
        snprintf(buffer, 128, "BENCHMARK: %.2f", fps);
        strcpy(widget->string, buffer);
    }
}


void start_campaign(int entity) {
    UNUSED(entity);
    strcpy(game_data->map_name, "Campaign");
    game_state = STATE_INTRO;
    reset_ids();
}


void load_campaign(int entity) {
    strcpy(game_data->map_name, "Campaign");
    change_state_load(entity);
    reset_ids();
}


void start_tutorial(int entity) {
    strcpy(game_data->map_name, "Tutorial");
    change_state_start(entity);
}


bool save_exists(String map_name) {
    // Make static to prevent stack overflow in Emscripten
    static String files[128];
    int files_count = list_files_alphabetically("save/*.json", files, NULL);
    return files_count > 0 && strcmp(files[0], map_name) == 0;
}


#ifdef __EMSCRIPTEN__
    #include <emscripten/emscripten.h>
    EMSCRIPTEN_KEEPALIVE
#endif
void create_menu() {
    int height = 8;

    if (game_settings.debug) {
        height += 2;
    }
    int container = create_container(vec(-18.0f, -2.0f), 1, height);
    WidgetComponent_get(container)->enabled = false;

    if (save_exists("Campaign")) {
        add_button_to_container(container, "CONTINUE", load_campaign);
    }

    add_button_to_container(container, "NEW GAME", start_campaign);
    add_button_to_container(container, "TUTORIAL", start_tutorial);
    add_button_to_container(container, "SURVIVAL", toggle_survival);
    #ifndef __EMSCRIPTEN__
        add_button_to_container(container, "LAN", toggle_lan);
    #endif
    if (game_settings.debug) {
        add_button_to_container(container, "EDITOR", toggle_editor);
    }
    add_button_to_container(container, "SETTINGS", toggle_settings);
    add_button_to_container(container, "CONTROLS", toggle_controls);
    add_button_to_container(container, "CREDITS", toggle_credits);
    if (game_settings.debug) {
        button_benchmark = add_button_to_container(container, "BENCHMARK", update_benchmark);
    }
    #ifndef __EMSCRIPTEN__
        add_button_to_container(container, "QUIT", change_state_quit);
    #endif
}


void destroy_menu() {
    for (int i = 0; i < game_data->components->menu_entities; i++) {
        int j = game_data->components->menu_entities_start + i;
        WidgetComponent* widget = WidgetComponent_get(j);
        if (widget) {
            destroy_entity(j);
        }
    }
    game_data->components->menu_entities = 0;
}


void create_pause_menu() {
    int height = game_data->game_mode == MODE_CAMPAIGN ? 5 : 3;
    int container = create_container(vec(-20.0f, 0.0f), 1, height);
    add_button_to_container(container, "RESUME", change_state_game);
    if (game_data->game_mode == MODE_CAMPAIGN) {
        add_button_to_container(container, "SAVE", change_state_save);
        add_button_to_container(container, "LOAD", change_state_load);
    }
    add_button_to_container(container, "SETTINGS", toggle_settings);
    add_button_to_container(container, "QUIT TO MENU", change_state_end);
}


void create_host_pause_menu() {
    int container = create_container(vec(-20.0f, 0.0f), 1, 3);
    add_button_to_container(container, "RESUME", change_state_host_game);
    add_button_to_container(container, "SETTINGS", toggle_settings);
    add_button_to_container(container, "RETURN TO LOBBY", change_state_lobby);
}


void create_client_pause_menu() {
    int container = create_container(vec(-20.0f, 0.0f), 1, 3);
    add_button_to_container(container, "RESUME", change_state_client_game);
    add_button_to_container(container, "SETTINGS", toggle_settings);
    add_button_to_container(container, "DISCONNECT", change_state_end);
}


void update_menu(float time_step) {
    update_server_list(time_step);
    update_lobby_menu();
    update_widgets(game_data->menu_camera);
}


void input_menu(int camera, SDL_Event event) {
    input_widgets(camera, event);
}


void draw_menu() {
    draw_widgets(game_data->menu_camera);

    Vector2f pos = get_mouse_position(game_data->menu_camera);
    draw_circle(game_data->menu_camera, pos, 0.1f, COLOR_WHITE);
}


void create_game_over_menu() {
    switch (network.mode) {
        case NET_MODE_HOST:
            create_button("RETURN TO LOBBY", vec(0.0f, -2.0f * BUTTON_HEIGHT), change_state_lobby);
            break;
        case NET_MODE_NONE:
            if (game_data->game_mode == MODE_CAMPAIGN && save_exists(game_data->map_name)) {
                create_button("LOAD SAVE", vec(0.0f, -1.0f * BUTTON_HEIGHT), change_state_load);
            } else {
                create_button("RESTART", vec(0.0f, -1.0f * BUTTON_HEIGHT), change_state_reset);
            }
            break;
        default:
            create_button("QUIT", vec(0.0f, -2.0f * BUTTON_HEIGHT), change_state_end);
    }
}


void create_win_menu() {
    float y = game_data->game_mode == MODE_DEATHMATCH ? -5.0f * BUTTON_HEIGHT : -1.0f * BUTTON_HEIGHT;
    switch (network.mode) {
        case NET_MODE_HOST:
            create_button("RETURN TO LOBBY", vec(0.0f, y), change_state_lobby);
            break;
        case NET_MODE_CLIENT:
            create_button("QUIT", vec(0.0f, y), change_state_end);
            break;
        default:
            create_button("CONTINUE", vec(0.0f, y), change_state_end);
    }
}


void set_map_data(Entity entity, int value) {
    WidgetComponent* widget = WidgetComponent_get(entity);
    strcpy(game_data->map_name, widget->strings[value]);
}


void set_point_limit(Entity entity, int value) {
    UNUSED(entity);
    game_data->point_limit = value;
}


void set_friendly_fire(Entity entity, int value) {
    UNUSED(entity);
    game_data->friendly_fire = value;
}


bool game_mode_is_selected(Filename map_name) {
    WidgetComponent* widget = WidgetComponent_get(lan_mode_dropdown);
    return get_map_game_mode(map_name) == widget->value;
}


void update_maps(Entity entity, int value) {
    UNUSED(entity);
    game_data->game_mode = value;

    String files[MAX_MAPS];
    int size = list_files_alphabetically("data/maps/*.json", files, game_mode_is_selected);
    set_widget_strings(lan_map_dropdown, files, size);
    set_map_data(lan_map_dropdown, 0);
}


void create_host_lobby_menu() {
    int height = 5;
    int container = create_container(vec(-18.0f, -2.0f), 2, height);
    WidgetComponent_get(container)->enabled = false;

    Entity mode_label = create_label("Mode", zeros());
    lan_mode_dropdown = create_dropdown(zeros(), GAME_MODES, 4);
    WidgetComponent* mode_widget = WidgetComponent_get(lan_mode_dropdown);
    mode_widget->on_change = update_maps;
    add_row_to_container(container, mode_label, lan_mode_dropdown);

    Entity map_label = create_label("Map", zeros());
    String files[MAX_MAPS];
    int size = list_files_alphabetically("data/maps/*.json", files, game_mode_is_selected);
    lan_map_dropdown = create_dropdown(zeros(), files, size);
    set_map_data(lan_map_dropdown, 0);
    WidgetComponent_get(lan_map_dropdown)->on_change = set_map_data;
    add_row_to_container(container, map_label, lan_map_dropdown);

    mode_widget->value = MODE_DEATHMATCH;
    update_maps(lan_mode_dropdown, MODE_DEATHMATCH);

    Entity friendly_fire_label = create_label("Friendly Fire", zeros());
    Entity friendly_fire_checkbox = create_checkbox(zeros(), game_data->friendly_fire, set_friendly_fire);
    add_row_to_container(container, friendly_fire_label, friendly_fire_checkbox);

    Entity points_label = create_label("Point limit", zeros());
    Entity point_limit = create_slider(vec(0.0f, 0.0f), 1, 20, 10, set_point_limit);
    add_row_to_container(container, points_label, point_limit);

    Entity start_button = create_button("Start game", zeros(), change_state_host_start);
    Entity close_button = create_button("Close lobby", zeros(), change_state_end);
    add_row_to_container(container, start_button, close_button);

    create_lobby_player_panel();
}


void create_lobby_menu() {
    int height = 5;
    int container = create_container(vec(-18.0f, -2.0f), 2, height);
    WidgetComponent_get(container)->enabled = false;

    Entity mode_label = create_label("Mode", zeros());
    client_lobby_mode_label = create_label("Waiting...", zeros());
    add_row_to_container(container, mode_label, client_lobby_mode_label);

    Entity map_label = create_label("Map", zeros());
    client_lobby_map_label = create_label("Waiting...", zeros());
    add_row_to_container(container, map_label, client_lobby_map_label);

    Entity friendly_fire_label = create_label("Friendly Fire", zeros());
    client_lobby_friendly_fire_label = create_label("Waiting...", zeros());
    add_row_to_container(container, friendly_fire_label, client_lobby_friendly_fire_label);

    Entity points_label = create_label("Point limit", zeros());
    client_lobby_point_limit_label = create_label("Waiting...", zeros());
    add_row_to_container(container, points_label, client_lobby_point_limit_label);

    Entity leave_button = create_button("Leave", zeros(), change_state_end);
    add_widget_to_container(container, leave_button);

    create_lobby_player_panel();
}
