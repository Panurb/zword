#include <stdio.h>
#include <time.h>

#include "game.h"
#include "camera.h"
#include "player.h"
#include "component.h"
#include "level.h"
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


ButtonText GAME_MODES[] = {
    "SURVIVAL",
    "CAMPAIGN",
    "TUTORIAL"
};

GameData* game_data;

static float game_over_timer = 0.0f;

// Survival
static int wave = 1;
static int enemies = 0;
static float wave_delay = 5.0f;
static List* spawners = NULL;
static float spawn_delay = 2.0f;



void change_state_game_over(GameData data) {
    game_over_timer = 2.0f;
    game_state = STATE_GAME_OVER;
    destroy_menu(data);
    create_game_over_menu(data);
}


void create_game(sfVideoMode mode) {
    ComponentData* components = ComponentData_create();
    ColliderGrid* grid = ColliderGrid_create();
    float ambient_light = 0.5f;
    int seed = time(NULL);
    int camera = create_camera(components, mode);
    int menu_camera = create_menu_camera(components, mode);

    sfTexture** textures = load_textures();
    sfSoundBuffer** sounds = load_sounds();

    sfRenderTexture* light_texture = sfRenderTexture_create(mode.width, mode.height, false);
    sfSprite* light_sprite = sfSprite_create();
    sfSprite_setTexture(light_sprite, sfRenderTexture_getTexture(light_texture), true);

    sfRenderTexture* shadow_texture = sfRenderTexture_create(mode.width, mode.height, false);
    sfSprite* shadow_sprite = sfSprite_create();
    sfSprite_setTexture(shadow_sprite, sfRenderTexture_getTexture(shadow_texture), true);

    game_data = malloc(sizeof(GameData));
    game_data->components = components;
    game_data->grid = grid;
    game_data->ambient_light = ambient_light;
    game_data->seed = seed;
    game_data->camera = camera;
    game_data->menu_camera = menu_camera;

    game_data->light_texture = light_texture;
    game_data->light_sprite = light_sprite;

    game_data->shadow_texture = shadow_texture;
    game_data->shadow_sprite = shadow_sprite;

    game_data->textures = textures;
    game_data->sounds = sounds;

    game_data->mode = mode;
}


void resize_game(GameData* data, sfVideoMode mode) {
    for (int i = 0; i < data->components->entities; i++) {
        CameraComponent* camera = CameraComponent_get(data->components, i);
        if (camera) {
            camera->resolution.x = mode.width;
            camera->resolution.y = mode.height;
            camera->zoom = camera->zoom_target * camera->resolution.y / 720.0;
        }
    }
    sfRenderTexture_destroy(data->light_texture);
    data->light_texture = sfRenderTexture_create(mode.width, mode.height, false);
    sfRenderTexture_destroy(data->shadow_texture);
    data->shadow_texture = sfRenderTexture_create(mode.width, mode.height, false);
}


void init_survival(GameData* data) {
    wave = 1;
    enemies = 0;
    wave_delay = 5.0f;
    spawn_delay = 2.0f;
    game_over_timer = 0.0f;

    spawners = List_create();
    for (int i = 0; i < data->components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(data->components, i);
        if (enemy && enemy->spawner) {
            List_add(spawners, i);
        }
    }
}


void start_game(GameData* data, Filename map_name) {
    ColliderGrid_clear(data->grid);
    CameraComponent* cam = CameraComponent_get(data->components, data->camera);
    sfVideoMode mode = { cam->resolution.x, cam->resolution.y, 32 };
    ComponentData_clear(data->components);
    data->camera = create_camera(data->components, mode);
    data->menu_camera = create_menu_camera(data->components, mode);
    data->ambient_light = 0.5f;
    create_pause_menu(data);
    // create_level(data.components, data.grid, data.seed);
    // test(data->components);
    load_game(data, map_name);

    init_grid(data->components, data->grid);
    init_survival(data);
}


void end_game(GameData* data) {
    ColliderGrid_clear(data->grid);
    CameraComponent* cam = CameraComponent_get(data->components, data->camera);
    sfVideoMode mode = { cam->resolution.x, cam->resolution.y, 32 };
    ComponentData_clear(data->components);
    data->camera = create_camera(data->components, mode);
    data->menu_camera = create_menu_camera(data->components, mode);
    create_menu(*data);
}


int spawn_enemy(ComponentData* components, ColliderGrid* grid, sfVector2f position, float probs[4]) {
    UNUSED(grid);
    int j = -1;
    float angle = rand_angle();
    switch (rand_choice(probs, 4)) {
        case 0:
            j = create_zombie(components, position, angle);
            break;
        case 1:
            j = create_farmer(components, position, angle);
            break;
        case 2:
            j = create_big_boy(components, position, angle);
            break;
        case 3:
            j = create_priest(components, position, angle);
            break;
    }
    int p = components->player.order->head->value;
    EnemyComponent* enemy = EnemyComponent_get(components, j);
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


int spawn_enemies(ComponentData* components, ColliderGrid* grid, int camera, float time_step, int max_enemies) {
    if (spawners && spawners->size == 0) {
        return 0;
    }

    static float delay = 0.0f;
    if (delay > 0.0f) {
        delay -= time_step;
    }

    int count = 0;
    for (int i = 0; i < components->entities; i++) {
        EnemyComponent* enemy = EnemyComponent_get(components, i);
        if (enemy && !enemy->spawner && enemy->state != ENEMY_DEAD) {
            count++;
        }
    }

    if (count < max_enemies && delay <= 0.0f) {
        int i = randi(0, spawners->size - 1);
        int spawner = List_get(spawners, i)->value;

        float x = randf(-1.0f, 1.0f);
        float y = randf(-1.0f, 1.0f);

        sfVector2f pos = get_position(components, spawner);
        pos = sum(pos, lin_comb(x, half_width(components, spawner), y, half_height(components, spawner)));

        if (on_screen(components, camera, pos, 2.0f, 2.0f)) {
            return 0;
        }

        float f = spawn_prob(1.0f, 0.1f, 1.0f);
        float b = spawn_prob(2.0f, 0.1f, 2.0f);
        float p = spawn_prob(3.0f, 0.1f, 3.0f);
        float probs[4] = {1.0f - f - b - p, f, b, p};
        
        spawn_enemy(components, grid, pos, probs);
        delay = 2.0f;
        return 1;
    }

    return 0;
}


void update_survival(GameData data, float time_step) {  
    bool players_alive = false;
    ListNode* node;
    FOREACH(node, data.components->player.order) {
        if (PlayerComponent_get(data.components, node->value)->state != PLAYER_DEAD) {
            players_alive = true;
            break;
        }
    }
    if (!players_alive) {
        change_state_game_over(data);
        List_delete(spawners);
        spawners = NULL;
        return;
    }

    if (wave_delay > 0.0f) {
        wave_delay -= time_step;
        return;
    }

    if (enemies < 10 + wave * 5) {
        enemies += spawn_enemies(data.components, data.grid, data.camera, time_step, 4 + wave);
    } else {
        bool enemies_alive = false;
        for (int i = 0; i < data.components->entities; i++) {
            EnemyComponent* enemy = EnemyComponent_get(data.components, i);
            if (enemy && !enemy->spawner && enemy->state != ENEMY_DEAD) {
                enemies_alive = true;
                break;
            }
        }
        PRINT(enemies_alive);
        if (!enemies_alive) {
            wave++;
            wave_delay = 5.0f;
            enemies = 0;
        }
    }
}


void update_game_mode(GameData data, float time_step) {
    switch (data.game_mode) {
        case MODE_SURVIVAL:
            update_survival(data, time_step);
            break;
        default:
            break;
    }
}


void draw_game_mode(GameData data, sfRenderWindow* window) {
    switch (data.game_mode) {
        case MODE_SURVIVAL:
            if (wave_delay > 0.0f) {
                char buffer[256];
                snprintf(buffer, 256, "WAVE %d", wave);
                draw_text(window, data.components, data.menu_camera, NULL, zeros(), buffer, 300, sfWhite);
            }
            break;
        default:
            break;
    }
}


void update_game(GameData data, float time_step) {
    update(data.components, time_step, data.grid);
    collide(data.components, data.grid);
    update_waypoints(data.components, data.grid, data.camera);
    update_doors(data.components);

    update_players(data.components, data.grid, time_step);
    update_weapons(data.components, time_step);
    update_enemies(data.components, data.grid, time_step);
    update_energy(data.components, data.grid);

    update_particles(data.components, data.camera, time_step);
    update_lights(data.components, time_step);
    update_camera(data.components, data.camera, time_step, true);

    draw_shadows(data.components, data.shadow_texture, data.camera);
    draw_lights(data.components, data.grid, data.light_texture, data.camera, data.ambient_light);

    animate(data.components, time_step);
}


void draw_game(GameData data, sfRenderWindow* window) {
    sfRenderStates state = { sfBlendMultiply, sfTransform_Identity, NULL, NULL };

    draw_ground(data.components, window, data.camera, data.textures);
    sfRenderWindow_drawSprite(window, data.shadow_sprite, &state);
    draw(data.components, window, data.camera, data.textures);
    sfRenderWindow_drawSprite(window, data.light_sprite, &state);

    draw_roofs(data.components, window, data.camera, data.textures);
    draw_items(&data, window);
}


void draw_parents(GameData data, sfRenderWindow* window) {
    for (int i = 0; i < data.components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(data.components, i);
        if (coord && coord->parent != -1) {
            sfVector2f start = get_position(data.components, i);
            sfVector2f end = get_position(data.components, coord->parent);
            draw_line(window, data.components, data.camera, NULL, start, end, 0.05f, get_color(0.0f, 1.0f, 1.0f, 0.5f));
        }
    }
}


void draw_entities(GameData data, sfRenderWindow* window) {
    char buffer[10];
    for (int i = 0; i < data.components->entities; i++) {
        CoordinateComponent* coord = CoordinateComponent_get(data.components, i);
        if (!coord) continue;

        snprintf(buffer, 10, "%d", i);
        draw_text(window, data.components, data.camera, NULL, coord->position, buffer, 20, sfWhite);
    }
}


void draw_debug(GameData data, sfRenderWindow* window, int debug_level) {
    draw_colliders(data.components, window, data.camera);
    draw_waypoints(data.components, window, data.camera, true);
    draw_enemies(data.components, window, data.camera);
    draw_parents(data, window);
    if (debug_level > 1) {
        draw_entities(data, window);
    }
    if (debug_level > 2) {
        draw_occupied_tiles(data.components, data.grid, window, data.camera);
    }
}


void update_game_over(GameData data, sfRenderWindow* window, float time_step) {
    if (game_over_timer > 0.0f) {
        update_game(data, time_step);
    }
    update_menu(data, window);
    game_over_timer = fmaxf(game_over_timer - time_step, 0.0f);
}


void draw_game_over(GameData data, sfRenderWindow* window) {
    draw_game(data, window);
    float alpha = 1.0f - game_over_timer / 2.0f;
    draw_overlay(window, data.components, data.menu_camera, alpha);
    sfColor color = get_color(1.0f, 0.0f, 0.0f, alpha);
    if (alpha == 1.0f) {
        draw_text(window, data.components, data.menu_camera, NULL, 
            vec(0.0f, 5.0f), "GAME OVER", 300, color);
        char buffer[256];
        snprintf(buffer, 256, "You survived until wave %d", wave);
        draw_text(window, data.components, data.menu_camera, NULL, 
            vec(0.0f, 1.0f), buffer, 40, sfWhite);
        draw_menu(data, window);
    }
}


int create_tutorial(ComponentData* components, sfVector2f position) {
    int entity = create_entity(components);
    CoordinateComponent_add(components, entity, position, 0.0f);
    ColliderComponent_add_circle(components, entity, 1.0f, GROUP_WALLS);
    TextComponent_add(components, entity, "", 30, sfWhite);

    return entity;
}


void draw_tutorials(sfRenderWindow* window, GameData data) {
    for (int i = 0; i < data.components->entities; i++) {
        TextComponent* text = TextComponent_get(data.components, i);
        if (text) {
            draw_text(window, data.components, data.camera, NULL, get_position(data.components, i), "?", 50, sfMagenta);
        }
    }
}
