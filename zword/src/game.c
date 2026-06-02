#include <stdio.h>
#include <time.h>
#include <string.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "app.h"
#include "game.h"
#include "camera.h"
#include "player.h"
#include "component.h"
#include "collider.h"
#include "physics.h"
#include "util.h"
#include "image.h"
#include "light.h"
#include "grid.h"
#include "enemy.h"
#include "particle.h"
#include "navigation.h"
#include "perlin.h"
#include "weapon.h"
#include "path.h"
#include "sound.h"
#include "hud.h"
#include "animation.h"
#include "door.h"
#include "menu.h"
#include "serialize.h"
#include "input.h"
#include "item.h"
#include "widget.h"
#include "settings.h"
#include "health.h"
#include "weather.h"
#include "network.h"
#include "netgame.h"


GameState game_state = STATE_MENU;
GameData* game_data;

Resources resources;

String GAME_MODES[] = {
    "SURVIVAL",
    "CAMPAIGN",
    "TUTORIAL",
    "DEATHMATCH"
};

String WEATHERS[] = {
    "NONE",
    "RAIN",
    "SNOW"
};


static float game_over_timer = 0.0f;
static bool level_won = false;
static MatchEndType match_end_type = MATCH_END_GAME_OVER;

// Survival
static int enemies = 0;
static List* spawners = NULL;
static float spawn_delay = 2.0f;

// Deathmatch
static Vector2f player_spawns[8];
static int player_spawns_count = 0;


static void send_match_end_packet(bool won) {
    if (network.mode != NET_MODE_HOST) {
        return;
    }

    EndGamePacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.header.type = PACKET_END_GAME;
    pkt.header.tick = network.tick;
    pkt.header.size = sizeof(EndGamePacket);
    pkt.end_type = won ? MATCH_END_WIN : MATCH_END_GAME_OVER;
    network_broadcast(&pkt, sizeof(pkt));
}


static void reset_player_controllers() {
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(node->value);
        if (!player) {
            continue;
        }

        reset_controller(&player->controller);
    }
}


void enter_match_end_screen(bool won) {
    game_over_timer = 2.0f;
    level_won = won;
    send_match_end_packet(won);
    reset_player_controllers();

    if (network.mode == NET_MODE_HOST) {
        game_state = STATE_HOST_GAME_OVER;
    } else if (network.mode == NET_MODE_CLIENT) {
        game_state = STATE_CLIENT_GAME_OVER;
    } else {
        game_state = STATE_GAME_OVER;
    }

    destroy_menu();
    if (won) {
        create_win_menu();
    } else {
        create_game_over_menu();
    }
}


void change_state_game_over() {
    if (game_data->testing) {
        game_state = STATE_LOAD_EDITOR;
        return;
    }

    enter_match_end_screen(false);
}


void change_state_win() {
    enter_match_end_screen(true);
}


void load_resources() {
    load_textures();
    LOG_INFO("Loaded %d textures", resources.textures_size);
    resources.fonts[0] = NULL;
    for (int size = 1; size <= 300; size++) {
        resources.fonts[size] = TTF_OpenFont("data/Helvetica.ttf", size);
        if (!resources.fonts[size]) {
            fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
            exit(1);
        }
    }
    LOG_INFO("Loaded %d fonts", 300);
    load_sounds();
    resources.music[0] = Mix_LoadMUS("data/music/zsong.ogg");
    resources.music[1] = Mix_LoadMUS("data/music/zsong2.ogg");
}


void create_game() {
    game_data = malloc(sizeof(GameData));

    strcpy(game_data->map_name, "");

    game_data->components = ComponentData_create();
    ColliderGrid* grid = ColliderGrid_create();
    float ambient_light = 0.5f;
    int seed = time(NULL);
    int camera = create_camera();
    int menu_camera = create_menu_camera();

    game_data->grid = grid;
    game_data->ambient_light = ambient_light;
    game_data->weather = WEATHER_NONE;
    game_data->wind = zeros();
    game_data->wind_speed = 1.0f;
    game_data->seed = seed;
    game_data->camera = camera;
    game_data->menu_camera = menu_camera;

    game_data->game_mode = MODE_SURVIVAL;
    game_data->testing = false;
    game_data->show_leaderboard = false;
    game_data->start_position = zeros();

    game_data->music = 0;
    game_data->point_limit = 0;
    game_data->wave = 1;
    game_data->wave_delay = 5.0f;
    game_data->friendly_fire = true;
}


void resize_game() {
    for (int i = 0; i < game_data->components->entities; i++) {
        CameraComponent* camera = CameraComponent_get(i);
        if (camera) {
            camera->resolution.w = game_settings.width;
            camera->resolution.h = game_settings.height;
            camera->zoom = camera->zoom_target * camera->resolution.h / 720.0;
        }
    }
}


void init_survival() {
    game_data->wave = 1;
    enemies = 0;
    game_data->wave_delay = 5.0f;
    spawn_delay = 2.0f;
    game_over_timer = 0.0f;
    level_won = false;

    spawners = List_create();
    for (int i = 0; i < game_data->components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(i);
        ItemComponent* item = ItemComponent_get(i);
        WeaponComponent* weapon = WeaponComponent_get(i);
        if (enemy && enemy->spawner) {
            List_add(spawners, i);
        }
        if (item && weapon && item->price > 0) {
            item->spawner = true;
            item->respawn_timer = 0.0f;
        }
    }
}


void init_tutorial() {
    game_over_timer = 0.0f;
    level_won = false;

    for (int i = 0; i < game_data->components->entities; i++) {
        ItemComponent* item = ItemComponent_get(i);
        if (item) {
            item->price = 0;
        }
    }
}


void init_deathmatch() {
    game_over_timer = 0.0f;
    level_won = false;

    for (int i = 0; i < game_data->components->entities; i++) {
        ItemComponent* item = ItemComponent_get(i);
        if (!item) continue;
        WeaponComponent* weapon = WeaponComponent_get(i);

        item->price = 0;
        if (weapon) {
            item->spawner = true;
            item->respawn_timer = 0.0f;
        }
    }
}


int get_player_count() {
    int player_count = 0;

    switch (network.mode) {
        case NET_MODE_NONE:
        case NET_MODE_CLIENT:
            for (int i = 0; i < 4; i++) {
                if (app.player_controllers[i] != CONTROLLER_NONE) {
                    player_count++;
                }
            }
            break;
        case NET_MODE_HOST:
            player_count = 1;  // host player
            for (int c = 0; c < NET_MAX_CLIENTS; c++) {
                if (network.clients[c].connected) {
                    player_count++;
                }
            }
            break;
    }

    return player_count;
}


void init_game() {
    player_spawns_count = 0;

    int player_count = get_player_count();
    int map_player_count = game_data->components->player.order->size;

    while (map_player_count < player_count) {
        Vector2f pos = get_position(game_data->components->player.order->head->value);
        pos = sum(pos, rand_vector());
        int entity = load_prefab("creatures/player", pos, 0.0f, ones());
        ColliderComponent* collider = ColliderComponent_get(entity);
        LOG_INFO("Not enough player spawns in map, created additional player %d at %f, %f", entity, pos.x, pos.y);
        if (collider) {
            LOG_INFO("Extra player %d host collider type=%d width=%.2f height=%.2f radius=%.2f", entity,
                collider->type, collider->width, collider->height, collider->radius);
        }
        map_player_count++;
    }

    int i = 0;
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        player_spawns[i] = get_position(node->value);
        player_spawns_count++;

        PlayerComponent* player = PlayerComponent_get(node->value);
        player->is_local = true;

        // In multiplayer, don't destroy remote players (they have CONTROLLER_NONE locally
        // but are controlled by remote clients)
        bool is_network_player = false;
        if (network.mode == NET_MODE_HOST && i > 0 && i < 4) {
            // On host: slots 1-3 may have connected clients
            for (int c = 0; c < NET_MAX_CLIENTS; c++) {
                if (network.clients[c].connected && network.clients[c].player_slot == i) {
                    is_network_player = true;
                    break;
                }
            }
        }
        if (app.player_controllers[i] == CONTROLLER_NONE && !is_network_player) {
            destroy_entity_recursive(node->value);
        } else {
            player->controller.joystick = app.player_controllers[i];
            if (network.mode == NET_MODE_HOST) {
                player->is_local = !is_network_player;
            } else if (network.mode == NET_MODE_CLIENT) {
                player->is_local = (i == network.local_player_slot);
            }
            LOG_DEBUG("Player %d: %d\n", i, player->controller.joystick);
        }

        i++;
    }

    init_grid();
    init_waypoints();

    switch (game_data->game_mode) {
        case MODE_SURVIVAL:
            init_survival();
            break;
        case MODE_TUTORIAL:
        case MODE_CAMPAIGN:
            init_tutorial();
            break;
        case MODE_DEATHMATCH:
            init_deathmatch();
            break;
        default:
            break;
    }

    init_weather();
}


void start_game(Filename map_name, bool load_save) {
    LOG_INFO("Starting game (%s)", map_name);

    ColliderGrid_clear(game_data->grid);
    ComponentData_clear();
    game_data->camera = create_camera();
    game_data->menu_camera = create_menu_camera();
    game_data->ambient_light = 0.5f;
    game_data->show_leaderboard = false;

    if (load_save) {
        load_state(map_name);
    } else {
        load_map(map_name);
    }

    init_game();
}


void set_player_names(String names[], int names_size) {
    int i = 0;
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(node->value);
        LOG_INFO("Setting player %d name to %s", i, names[i]);
        strncpy(player->name, names[i], sizeof(player->name) - 1);
        i++;
        if (i >= names_size) {
            break;
        }
    }
}


void end_game() {
    ColliderGrid_clear(game_data->grid);
    ComponentData_clear();
    game_data->camera = create_camera();
    game_data->menu_camera = create_menu_camera();
    game_data->show_leaderboard = false;
    create_menu();
}


void end_match() {
    ColliderGrid_clear(game_data->grid);
    ComponentData_clear();
    game_data->camera = create_camera();
    game_data->menu_camera = create_menu_camera();
    game_data->show_leaderboard = false;
}


float spawn_prob(float min_wave, float max_prob, float rate) {
    if (game_data->wave + rate - min_wave == 0.0f) return 0.0f;
    return fmaxf(0.0f, max_prob * (1 - rate / (game_data->wave + rate - min_wave)));
}


int spawn_enemies(int camera, float time_step, int max_enemies) {
    if (spawners && spawners->size == 0) {
        return 0;
    }

    if (spawn_delay > 0.0f) {
        spawn_delay -= time_step;
    }

    int count = 0;
    for (int i = 0; i < game_data->components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(i);
        if (enemy && !enemy->spawner && enemy->state != ENEMY_DEAD) {
            count++;
        }
    }

    if (count < max_enemies && spawn_delay <= 0.0f) {
        int i = randi(0, spawners->size - 1);
        int spawner = List_get(spawners, i)->value;

        float x = randf(-1.0f, 1.0f);
        float y = randf(-1.0f, 1.0f);

        Vector2f pos = get_position(spawner);
        pos = sum(pos, lin_comb(x, half_width(spawner), y, half_height(spawner)));

        if (on_screen(camera, pos, 2.0f, 2.0f)) {
            return 0;
        }

        float f = spawn_prob(1.0f, 0.1f, 1.0f);
        float b = spawn_prob(2.0f, 0.1f, 2.0f);
        float p = spawn_prob(3.0f, 0.1f, 3.0f);
        float probs[4] = {1.0f - f - b - p, f, b, p};
        
        spawn_enemy(pos, probs);
        spawn_delay = 2.0f;
        return 1;
    }

    return 0;
}


void update_survival(float time_step) {  
    bool players_alive = false;
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        if (PlayerComponent_get(node->value)->state != PLAYER_DEAD) {
            players_alive = true;
            break;
        }
    }
    if (!players_alive) {
        change_state_game_over();
        List_delete(spawners);
        spawners = NULL;
        return;
    }

    if (game_data->wave_delay > 0.0f) {
        game_data->wave_delay -= time_step;
        return;
    }

    if (enemies < 10 + game_data->wave * 5) {
        enemies += spawn_enemies(game_data->camera, time_step, 4 + game_data->wave);
    } else {
        bool enemies_alive = false;
        for (int i = 0; i < game_data->components->entities; i++) {
            EnemyComponent* enemy = EnemyComponent_get(i);
            if (enemy && !enemy->spawner && enemy->state != ENEMY_DEAD) {
                enemies_alive = true;
                break;
            }
        }

        if (!enemies_alive) {
            int i = 0;
            FOREACH(node, game_data->components->player.order) {
                PlayerComponent* player = PlayerComponent_get(node->value);
                if (player->state == PLAYER_DEAD && i < player_spawns_count) {
                    respawn_player(node->value, player_spawns[i]);
                    player->money = 0;
                    player->money_increment = 0;
                    player->money_timer = 0.0f;
                }

                i++;
            }

            game_data->wave++;
            game_data->wave_delay = 5.0f;
            enemies = 0;
        }
    }
}


void update_campaign(float time_step) {
    UNUSED(time_step);

    bool players_alive = false;
    bool players_won = true;

    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(node->value);
        if (player->state != PLAYER_DEAD) {
            players_alive = true;
            
            if (!player->won) {
                players_won = false;
            }
        }
        player->won = false;
    }

    if (!players_alive) {
        change_state_game_over();
        return;
    }

    if (players_won) {
        bool boss_alive = false;
        for (Entity i = 0; i < game_data->components->entities; i++) {
            EnemyComponent* enemy = EnemyComponent_get(i);
            if (enemy && enemy->boss && enemy->state != ENEMY_DEAD) {
                boss_alive = true;
                break;
            }
        }

        if (!boss_alive) {
            change_state_win();
        }
    }
}


void update_tutorial(float time_step) {
    UNUSED(time_step);

    bool players_alive = false;
    bool players_won = true;

    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(node->value);
        if (player->state != PLAYER_DEAD) {
            players_alive = true;
            
            if (!player->won) {
                players_won = false;
            }
        }
        player->won = false;
    }

    if (!players_alive) {
        change_state_game_over();
        return;
    }

    if (players_won) {
        change_state_win();
        return;
    }
}


float get_nearest_player_distance(Vector2f position) {
    float min_dist = INFINITY;
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(node->value);
        if (player->state != PLAYER_DEAD) {
            Vector2f player_pos = get_position(node->value);
            float d = dist(position, player_pos);
            if (d < min_dist) {
                min_dist = d;
            }
        }
    }
    return min_dist;
}


void update_deathmatch_weapons(float time_step) {
    for (int i = 0; i < game_data->components->entities; i++) {
        ItemComponent* item = ItemComponent_get(i);
        WeaponComponent* weapon = WeaponComponent_get(i);
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!item || !weapon || !coord) continue;
        if (!item->spawner) continue;
        if (item->respawn_timer <= 0.0f) continue;

        item->respawn_timer = fmaxf(item->respawn_timer - time_step, 0.0f);
        if (item->respawn_timer > 0.0f) continue;

        ColliderComponent* collider = ColliderComponent_get(i);
        ImageComponent* image = ImageComponent_get(i);
        if (collider) {
            collider->enabled = true;
            update_grid(i);
        }
        if (image) {
            image->alpha = 1.0f;
        }
    }
}


void update_deathmatch(float time_step) {
    update_deathmatch_weapons(time_step);

    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(node->value);

        if (player->kills >= game_data->point_limit) {
            change_state_win();
            return;
        }

        if (player->state == PLAYER_DEAD && player->respawn_timer == 0.0f) {
            float max_dist = 0.0f;
            Vector2f spawn_pos = zeros();

            for (int i = 0; i < player_spawns_count; i++) {
                float d = get_nearest_player_distance(player_spawns[i]);
                if (d > max_dist) {
                    max_dist = d;
                    spawn_pos = player_spawns[i];
                }
            }

            respawn_player(node->value, spawn_pos);
        }
    }
}


void update_game_mode(float time_step) {
    switch (game_data->game_mode) {
        case MODE_SURVIVAL:
            update_survival(time_step);
            break;
        case MODE_CAMPAIGN:
            update_campaign(time_step);
            break;
        case MODE_TUTORIAL:
            update_tutorial(time_step);
            break;
        case MODE_DEATHMATCH:
            update_deathmatch(time_step);
            break;
        default:
            break;
    }
}


void draw_game_mode() {
    switch (game_data->game_mode) {
        case MODE_SURVIVAL:
            if (game_data->wave_delay > 0.0f) {
                char buffer[256];
                snprintf(buffer, 256, "WAVE %d", game_data->wave);
                draw_text(game_data->menu_camera, zeros(), buffer, 300, COLOR_WHITE);
            }
            break;
        default:
            break;
    }
}


void update_lifetimes(float time_step) {
    for (int i = 0; i < game_data->components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;

        if (coord->lifetime > 0.0f) {
            coord->lifetime = fmaxf(0.0f, coord->lifetime - time_step);

            ImageComponent* image = ImageComponent_get(i);
            if (coord->lifetime < 1.0f) {
                if (image) {
                    image->alpha = fminf(image->alpha, coord->lifetime);
                }
            }
        } else if (coord->lifetime == 0.0f) {
            LightComponent* light = LightComponent_get(i);
            if (light) {
                light->enabled = false;
            }

            ParticleComponent* particle = ParticleComponent_get(i);
            if (particle) {
                particle->loop = false;
                particle->enabled = false;
                if (particle->particles > 0) {
                    continue;
                }
            }
            if (ColliderComponent_get(i)) {
                clear_grid(i);
            }
            if (SoundComponent_get(i)) {
                clear_sounds(i);
            }
            destroy_entity_recursive(i);
        }
    }
}


void update_coordinates() {
    for (int i = 0; i < game_data->components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (coord) {
            coord->previous.position = get_position(i);
            coord->previous.angle = get_angle(i);
            coord->previous.scale = get_scale(i);
        }
    }

    for (int i = 0; i < game_data->components->menu_entities; i++) {
        int j = game_data->components->menu_entities_start + i;
        CoordinateComponent* coord = CoordinateComponent_get(j);
        if (coord) {
            coord->previous.position = get_position(j);
            coord->previous.angle = get_angle(j);
            coord->previous.scale = get_scale(j);
        }
    }
}


void update_game(float time_step) {
    update_coordinates();

    update_lifetimes(time_step);
    update_health(time_step);

    update_physics(time_step);
    update_collisions();
    update_waypoints();
    update_doors();

    update_players(time_step);
    update_weapons(time_step);
    update_enemies(time_step);
    update_energy();

    update_particles(game_data->camera, time_step);
    update_lights(time_step);
    update_camera(game_data->camera, time_step, true);
    update_weather(time_step);

    animate(time_step);
}


void draw_game() {
    draw_ground(game_data->camera);
    SDL_RenderCopy(app.renderer, app.shadow_texture, NULL, NULL);
    draw_images(game_data->camera);
    draw_particles(game_data->camera);
    SDL_RenderCopy(app.renderer, app.light_texture, NULL, NULL);
    draw_roofs(game_data->camera);
    draw_respawning_weapons();
    draw_player_targets();

    draw_shadows(game_data->camera);
    draw_lights(game_data->camera, game_data->ambient_light);
}


void draw_parents() {
    for (int i = 0; i < game_data->components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;
        
        Vector2f start = get_position(i);

        String buffer;
        snprintf(buffer, sizeof(buffer), "%d", coord->parent);
        draw_text(game_data->camera, sum(start, vec(0.0f, 1.0f)), buffer, 20, COLOR_RED);
        
        if (coord->parent != -1) {
            Vector2f end = get_position(coord->parent);
            draw_line(game_data->camera, start, end, 0.05f, get_color(0.0f, 1.0f, 1.0f, 0.5f));
        }
    }
}


void draw_entities() {
    char buffer[10];
    for (int i = 0; i < game_data->components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (!coord) continue;

        snprintf(buffer, 10, "%d", i);
        draw_text(game_data->camera, coord->position, buffer, 20, COLOR_WHITE);
    }
}


void draw_debug(int debug_level) {
    draw_line(game_data->menu_camera, zeros(), game_data->wind, 0.5f, COLOR_BLUE);
    draw_colliders(game_data->camera);
    draw_waypoints(game_data->camera, true);
    draw_enemies(game_data->camera);
    draw_parents();
    draw_vectors(game_data->camera);
    if (debug_level > 1) {
        draw_entities();
    }
    if (debug_level > 2) {
        draw_occupied_tiles(game_data->camera);
    }
    for (int i = 0; i < game_data->components->entities; i++) {
        DoorComponent* door = DoorComponent_get(i);
        if (door) {
            draw_text(game_data->camera, get_position(i), door->locked ? "locked" : "unlocked", 20, COLOR_WHITE);
        }
    }
}


void update_game_over(float time_step) {
    if (game_over_timer > 0.0f) {
        update_game(time_step);
    } else {
        update_menu();
    }
    game_over_timer = fmaxf(game_over_timer - time_step, 0.0f);
}


Entity get_winner() {
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(node->value);
        if (player->kills >= game_data->point_limit) {
            return node->value;
        }
    }
    return NULL_ENTITY;
}


void draw_game_over() {
    draw_game();
    float alpha = 1.0f - game_over_timer / 2.0f;
    draw_overlay(game_data->menu_camera, alpha);
    Color color = get_color(1.0f, 0.0f, 0.0f, alpha);
    if (alpha == 1.0f) {
        if (level_won) {
            if (game_data->game_mode == MODE_DEATHMATCH) {
                PlayerComponent* player = PlayerComponent_get(get_winner());
                String buffer;
                snprintf(buffer, STRING_SIZE, "%s WINS", player->name);
                draw_text(game_data->menu_camera, vec(0.0f, 8.0f), buffer, 300, color);
                draw_leaderboard(game_data->menu_camera);
            } else {
                draw_text(game_data->menu_camera, vec(0.0f, 5.0f), "YOU WON", 300, color);
            }
        } else {
            draw_text(game_data->menu_camera, vec(0.0f, 5.0f), "GAME OVER", 300, color);
        }
        if (game_data->game_mode == MODE_SURVIVAL) {
            char buffer[256];
            snprintf(buffer, 256, "You survived until wave %d", game_data->wave);
            draw_text(game_data->menu_camera, vec(0.0f, 1.0f), buffer, 40, color);
        }
        draw_menu();
    }
}


int create_tutorial(Vector2f position) {
    int entity = create_entity();
    CoordinateComponent_add(entity, position, 0.0f);
    ColliderComponent_add_circle(entity, 1.0f, GROUP_DEBRIS);
    TextComponent_add(entity, "", 30, COLOR_WHITE);

    return entity;
}


int create_level_end(Vector2f position, float angle, float width, float height) {
    int entity = create_entity();
    CoordinateComponent_add(entity, position, angle);
    ColliderComponent* collider = ColliderComponent_add_rectangle(entity, width, height, GROUP_WALLS);
    collider->trigger_type = TRIGGER_WIN;

    return entity;
}


void draw_tutorials() {
    for (int i = 0; i < game_data->components->entities; i++) {
        TextComponent* text = TextComponent_get(i);
        if (text) {
            draw_text(game_data->camera, get_position(i), "?", 50, COLOR_MAGENTA);
        }

        ColliderComponent* collider = ColliderComponent_get(i);
        if (collider && collider->trigger_type == TRIGGER_WIN) {
            Vector2f pos = get_position(i);
            Color color = get_color(0.0f, 1.0f, 0.0f, 0.25f);
            float angle = get_angle(i);
            draw_rectangle(game_data->camera, pos, collider_width(i), collider_height(i), angle, 
                color);
            draw_text(game_data->camera, pos, "level_end", 20, COLOR_GREEN);
        }
    }
}
