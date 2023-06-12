#include "game.h"
#include "camera.h"
#include "player.h"
#include "component.h"
#include "level.h"
#include "collider.h"
#include "physics.h"
#include "util.h"
#include <time.h>

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
        light_texture, light_sprite, shadow_texture, shadow_sprite, mode, "" };
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


void start_game(GameData data) {
    ColliderGrid_clear(data.grid);
    CameraComponent* cam = CameraComponent_get(data.components, data.camera);
    sfVideoMode mode = { cam->resolution.x, cam->resolution.y, 32 };
    ComponentData_clear(data.components);
    data.camera = create_camera(data.components, mode);
    create_pause_menu(data);
    // create_level(data.components, data.grid, data.seed);
    test(data.components, data.grid);
    init_grid(data.components, data.grid);
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

    // spawn_enemies(components, grid, camera);
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


void draw_debug(GameData data, sfRenderWindow* window, int debug_level) {
    draw_colliders(data.components, window, data.camera);
    draw_waypoints(data.components, window, data.camera, true);
    draw_enemies(data.components, window, data.camera);
    if (debug_level > 1) {
        draw_occupied_tiles(data.components, data.grid, window, data.camera);
    }
}
