#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <SFML/Audio.h>
#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>
#include <SFML/Window/Keyboard.h>

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
#include "interface.h"
#include "perlin.h"
#include "weapon.h"
#include "road.h"
#include "sound.h"
#include "hud.h"
#include "animation.h"
#include "door.h"


int main() {
    sfVideoMode mode = { 1920, 1080, 32 };
    sfContext* context = sfContext_create();
    sfContextSettings settings = sfContext_getSettings(context);
    settings.antialiasingLevel = 8;
    sfRenderWindow* window = sfRenderWindow_create(mode, "zword", sfClose, &settings);
    // sfRenderWindow* window = sfRenderWindow_create(mode, "zword", sfFullscreen, &settings);
    sfRenderWindow_setKeyRepeatEnabled(window, sfFalse);
    sfRenderWindow_setMouseCursorVisible(window, false);
    // sfWindow_setVerticalSyncEnabled((sfWindow*) window, true);
    // sfWindow_setFramerateLimit((sfWindow*) window, 60);
    bool focus = true;
    sfJoystick_update();
    
    sfTexture** textures = load_textures();
    sfSoundBuffer** sounds = load_sounds();

    sfSound* channels[MAX_SOUNDS];
    for (int i = 0; i < MAX_SOUNDS; i++) {
        channels[i] = sfSound_create();
    }

    sfRenderStates state = { sfBlendMultiply, sfTransform_Identity, NULL, NULL };
    sfRenderTexture* light_texture = sfRenderTexture_create(mode.width, mode.height, sfFalse);
    sfSprite* light_sprite = sfSprite_create();
    sfSprite_setTexture(light_sprite, sfRenderTexture_getTexture(light_texture), sfTrue);

    FpsCounter* fps = FpsCounter_create();

    sfClock* clock = sfClock_create();
    float time_step = 1.0f / 60.0f;
    float elapsed_time = 0.0f;

    ComponentData* components = ComponentData_create();
    ColliderGrid* grid = ColliderGrid_create();

    float ambient_light = 0.4f;
    int seed = time(NULL);

    int camera = create_camera(components, mode);
    // create_level(components, grid, seed);
    test(components, grid);
    init_grid(components, grid);

    while (sfRenderWindow_isOpen(window)) {
        float delta_time = sfTime_asSeconds(sfClock_restart(clock));

        sfEvent event;
        while (sfRenderWindow_pollEvent(window, &event)) {
            switch (event.type) {
                case sfEvtLostFocus:
                    focus = false;
                    break;
                case sfEvtGainedFocus:
                    focus = true;
                    sfClock_restart(clock);
                    break;
                case sfEvtClosed:
                    sfRenderWindow_close(window);
                    break;
                case sfEvtKeyPressed:
                    if (event.key.code == sfKeyEscape) {
                        sfRenderWindow_close(window);
                    } else if (event.key.code == sfKeyF5 || event.key.code == sfKeyF6) {
                        if (event.key.code == sfKeyF6) {
                            seed = time(NULL);
                        }

                        ColliderGrid_clear(grid);
                        ComponentData_clear(components);

                        camera = create_camera(components, mode);
                        create_level(components, grid, seed);
                        init_grid(components, grid);
                        sfClock_restart(clock);
                    }
                    break;
                default:
                    break;
            }
        }

        if (focus) {
            while (elapsed_time > time_step) {
                elapsed_time -= time_step;

                input(components, window, camera);

                update(components, time_step, grid);
                collide(components, grid);
                update_waypoints(components, grid, camera);
                update_doors(components, grid);

                update_players(components, grid, time_step);
                update_weapons(components, time_step);
                update_enemies(components, grid);
                update_energy(components, grid);

                update_particles(components, camera, time_step);
                update_lights(components, time_step);
                update_camera(components, camera, time_step);

                draw_lights(components, grid, light_texture, camera, ambient_light);

                animate(components, time_step);
            }

            elapsed_time += delta_time;
        }

        sfRenderWindow_clear(window, sfBlack);

        draw(components, window, camera, textures);
        sfRenderWindow_drawSprite(window, light_sprite, &state);
        draw_roofs(components, window, camera, textures);
        draw_outlines(components, window, camera);
        draw_hud(components, window, camera);

        // draw_colliders(components, window, camera);
        // draw_waypoints(components, window, camera);
        // draw_enemies(components, window, camera);
        // draw_grid(grid, window, camera);
        // draw_occupied_tiles(components, grid, window, camera);
        draw_fps(window, fps, delta_time);

        sfRenderWindow_display(window);

        play_sounds(components, camera, sounds, channels);
    }

    sfRenderWindow_destroy(window);

    return 0;
}
