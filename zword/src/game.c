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
#include "road.h"
#include "sound.h"
#include "hud.h"
#include "animation.h"
#include "door.h"
#include "menu.h"
#include "serialize.h"
#include "item.h"
#include "widget.h"
#include "settings.h"
#include "health.h"
#include "weather.h"


GameState game_state = STATE_MENU;
GameData* game_data;

Resources resources;

ButtonText GAME_MODES[] = {
    "SURVIVAL",
    "CAMPAIGN",
    "TUTORIAL"
};

ButtonText WEATHERS[] = {
    "NONE",
    "RAIN",
    "SNOW"
};


static float game_over_timer = 0.0f;
static bool level_won = false;

// Survival
static int wave = 1;
static int enemies = 0;
static float wave_delay = 5.0f;
static List* spawners = NULL;
static float spawn_delay = 2.0f;



void change_state_game_over() {
    game_over_timer = 2.0f;
    game_state = STATE_GAME_OVER;
    level_won = false;
    destroy_menu();
    create_game_over_menu();
}


void change_state_win() {
    game_over_timer = 2.0f;
    game_state = STATE_GAME_OVER;
    level_won = true;
    destroy_menu();
    create_win_menu();
}


void load_resources() {
    load_textures();
    resources.fonts[0] = NULL;
    for (int size = 1; size <= 300; size++) {
        resources.fonts[size] = TTF_OpenFont("data/Helvetica.ttf", size);
        if (!resources.fonts[size]) {
            fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
            exit(1);
        }
    }
    load_sounds();
    resources.music[0] = Mix_LoadMUS("data/music/zsong.ogg");
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
    wave = 1;
    enemies = 0;
    wave_delay = 5.0f;
    spawn_delay = 2.0f;
    game_over_timer = 0.0f;
    level_won = false;

    spawners = List_create();
    for (int i = 0; i < game_data->components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(i);
        if (enemy && enemy->spawner) {
            List_add(spawners, i);
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


void start_game(Filename map_name) {
    ColliderGrid_clear(game_data->grid);
    ComponentData_clear();
    game_data->camera = create_camera();
    game_data->menu_camera = create_menu_camera();
    game_data->ambient_light = 0.5f;
    create_pause_menu();
    // create_level(data.components, data.grid, data.seed);
    // test(data->components);
    load_game(map_name);

    int i = 0;
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        if (app.player_controllers[i] == CONTROLLER_NONE) {
            destroy_entity_recursive(node->value);
            continue;
        }

        PlayerComponent* player = PlayerComponent_get(node->value);
        player->controller.joystick = app.player_controllers[i];
        LOG_DEBUG("Player %d: %d\n", i, player->controller.joystick);
        i++;
    }

    init_grid();
    init_waypoints();

    switch (game_data->game_mode) {
        case MODE_SURVIVAL:
            init_survival();
            break;
        case MODE_TUTORIAL:
            init_tutorial();
            break;
        default:
            break;
    }

    init_weather();
}


void end_game() {
    ColliderGrid_clear(game_data->grid);
    ComponentData_clear();
    game_data->camera = create_camera();
    game_data->menu_camera = create_menu_camera();
    create_menu();
}


int spawn_enemy(Vector2f position, float probs[4]) {
    int j = -1;
    float angle = rand_angle();
    switch (rand_choice(probs, 4)) {
        case 0:
            j = create_zombie(position, angle);
            break;
        case 1:
            j = create_farmer(position, angle);
            break;
        case 2:
            j = create_big_boy(position, angle);
            break;
        case 3:
            j = create_priest(position, angle);
            break;
    }
    int p = game_data->components->player.order->head->value;
    EnemyComponent* enemy = EnemyComponent_get(j);
    if (enemy) {
        enemy->target = p;
        enemy->state = ENEMY_CHASE;
        spawn_delay = 2.0f;
        // TODO: update grid?
    }

    return j;
}


float spawn_prob(float min_wave, float max_prob, float rate) {
    if (wave + rate - min_wave == 0.0f) return 0.0f;
    return fmaxf(0.0f, max_prob * (1 - rate / (wave + rate - min_wave)));
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

    if (wave_delay > 0.0f) {
        wave_delay -= time_step;
        return;
    }

    if (enemies < 10 + wave * 5) {
        enemies += spawn_enemies(game_data->camera, time_step, 4 + wave);
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
            wave++;
            wave_delay = 5.0f;
            enemies = 0;
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


void update_game_mode(float time_step) {
    switch (game_data->game_mode) {
        case MODE_SURVIVAL:
            update_survival(time_step);
            break;
        case MODE_TUTORIAL:
            update_tutorial(time_step);
            break;
        default:
            break;
    }
}


void draw_game_mode() {
    switch (game_data->game_mode) {
        case MODE_SURVIVAL:
            if (wave_delay > 0.0f) {
                char buffer[256];
                snprintf(buffer, 256, "WAVE %d", wave);
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
    draw_player_targets();

    draw_shadows(game_data->camera);
    draw_lights(game_data->camera, game_data->ambient_light);
}


void draw_parents() {
    for (int i = 0; i < game_data->components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (coord && coord->parent != -1) {
            Vector2f start = get_position(i);
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


void draw_game_over() {
    draw_game();
    float alpha = 1.0f - game_over_timer / 2.0f;
    draw_overlay(game_data->menu_camera, alpha);
    Color color = get_color(1.0f, 0.0f, 0.0f, alpha);
    if (alpha == 1.0f) {
        if (level_won) {
            draw_text(game_data->menu_camera, vec(0.0f, 5.0f), "YOU WON", 300, color);
        } else {
            draw_text(game_data->menu_camera, vec(0.0f, 5.0f), "GAME OVER", 300, color);
        }
        if (game_data->game_mode == MODE_SURVIVAL) {
            char buffer[256];
            snprintf(buffer, 256, "You survived until wave %d", wave);
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
