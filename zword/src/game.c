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


static float game_over_timer = 0.0f;

// Survival
static int wave = 1;
static int enemies = 0;
static float wave_delay = 5.0f;



void change_state_game_over(GameData data) {
    game_over_timer = 2.0f;
    game_state = STATE_GAME_OVER;
    destroy_menu(data);
    create_game_over_menu(data);
}


GameData create_game(sfVideoMode mode) {
    ComponentData* components = ComponentData_create();
    ColliderGrid* grid = ColliderGrid_create();
    float ambient_light = 0.4f;
    int seed = time(NULL);
    int camera = create_camera(components, mode);
    int menu_camera = create_camera(components, mode);

    sfTexture** textures = load_textures();
    sfSoundBuffer** sounds = load_sounds();

    sfRenderTexture* light_texture = sfRenderTexture_create(mode.width, mode.height, false);
    sfSprite* light_sprite = sfSprite_create();
    sfSprite_setTexture(light_sprite, sfRenderTexture_getTexture(light_texture), true);

    sfRenderTexture* shadow_texture = sfRenderTexture_create(mode.width, mode.height, false);
    sfSprite* shadow_sprite = sfSprite_create();
    sfSprite_setTexture(shadow_sprite, sfRenderTexture_getTexture(shadow_texture), true);

    GameData data = { textures, sounds, components, grid, ambient_light, seed, camera, menu_camera,
        light_texture, light_sprite, shadow_texture, shadow_sprite, mode, "", MODE_SURVIVAL };
    return data;
}


void resize_game(GameData* data, sfVideoMode mode) {
    CameraComponent* camera = CameraComponent_get(data->components, data->camera);
    camera->resolution.x = mode.width;
    camera->resolution.y = mode.height;
    camera->zoom_target = 25.0f;
    camera->zoom = camera->zoom_target * camera->resolution.y / 720.0;
    sfRenderTexture_destroy(data->light_texture);
    data->light_texture = sfRenderTexture_create(mode.width, mode.height, false);
    sfRenderTexture_destroy(data->shadow_texture);
    data->shadow_texture = sfRenderTexture_create(mode.width, mode.height, false);
}


void start_game(GameData* data, Filename map_name) {
    ColliderGrid_clear(data->grid);
    CameraComponent* cam = CameraComponent_get(data->components, data->camera);
    sfVideoMode mode = { cam->resolution.x, cam->resolution.y, 32 };
    ComponentData_clear(data->components);
    data->camera = create_camera(data->components, mode);
    data->menu_camera = create_camera(data->components, mode);
    create_pause_menu(data);
    // create_level(data.components, data.grid, data.seed);
    // test(data->components, data->grid);
    load_game(data, map_name);
    init_grid(data->components, data->grid);
}


void end_game(GameData* data) {
    ColliderGrid_clear(data->grid);
    CameraComponent* cam = CameraComponent_get(data->components, data->camera);
    sfVideoMode mode = { cam->resolution.x, cam->resolution.y, 32 };
    ComponentData_clear(data->components);
    data->camera = create_camera(data->components, mode);
    data->menu_camera = create_camera(data->components, mode);
    create_menu(*data);
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
        if (!enemies_alive) {
            wave++;
            wave_delay = 5.0f;
            enemies = 0;
        }
    }
}


void update_game_mode(GameData data, sfRenderWindow* window, float time_step) {
    UNUSED(window);

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
                draw_text(window, data.components, data.menu_camera, NULL, zeros(), buffer, sfWhite);
            }
            break;
        default:
            break;
    }
}


void update_game(GameData data, sfRenderWindow* window, float time_step) {
    update(data.components, time_step, data.grid);
    collide(data.components, data.grid);
    update_waypoints(data.components, data.grid, data.camera);
    update_doors(data.components);

    update_players(data.components, data.grid);
    update_weapons(data.components, time_step);
    update_enemies(data.components, data.grid, time_step);
    update_energy(data.components, data.grid);

    update_particles(data.components, data.camera, time_step);
    update_lights(data.components, time_step);
    update_camera(data.components, data.camera, time_step, true);

    draw_shadows(data.components, data.shadow_texture, data.camera);
    draw_lights(data.components, data.grid, data.light_texture, data.camera, data.ambient_light);

    animate(data.components, time_step);

    update_game_mode(data, window, time_step);
}


void draw_game(GameData data, sfRenderWindow* window) {
    sfRenderStates state = { sfBlendMultiply, sfTransform_Identity, NULL, NULL };

    draw_ground(data.components, window, data.camera, data.textures);
    sfRenderWindow_drawSprite(window, data.shadow_sprite, &state);
    draw(data.components, window, data.camera, data.textures);
    sfRenderWindow_drawSprite(window, data.light_sprite, &state);

    draw_roofs(data.components, window, data.camera, data.textures);
    draw_outlines(data.components, window, data.camera);
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
        draw_text(window, data.components, data.camera, NULL, coord->position, buffer, sfWhite);
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
    update_menu(data, window);
    game_over_timer = fmaxf(game_over_timer - time_step, 0.0f);
}


void draw_game_over(GameData data, sfRenderWindow* window) {
    draw_game(data, window);
    float alpha = 1.0f - game_over_timer / 2.0f;
    draw_overlay(window, data.components, data.menu_camera, alpha);
    sfColor color = get_color(1.0f, 0.0f, 0.0f, alpha);
    draw_text(window, data.components, data.menu_camera, NULL, zeros(), "GAME OVER", color);
    if (alpha == 1.0f) {
        draw_menu(data, window);
    }
}
