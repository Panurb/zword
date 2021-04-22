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


int main() {
    sfVideoMode mode = { 1920, 1080, 32 };
    sfContext* context = sfContext_create();
    sfContextSettings settings = sfContext_getSettings(context);
    settings.antialiasingLevel = 8;
    sfRenderWindow* window = sfRenderWindow_create(mode, "zword", sfClose, &settings);
    sfRenderWindow_setKeyRepeatEnabled(window, sfFalse);
    // sfWindow_setVerticalSyncEnabled((sfWindow*) window, true);
    // sfWindow_setFramerateLimit((sfWindow*) window, 60);
    bool focus = true;
    sfJoystick_update();
    
    sfTexture* textures = load_textures();
    sfSoundBuffer* sounds = load_sounds();

    sfSound* channels[MAX_SOUNDS];
    for (int i = 0; i < MAX_SOUNDS; i++) {
        channels[i] = sfSound_create();
        sfSound_setAttenuation(channels[i], 0.1);
        sfSound_setRelativeToListener(channels[i], true);
    }

    sfRenderStates state = { sfBlendMultiply, sfTransform_Identity, NULL, NULL };
    sfRenderTexture* light_texture = sfRenderTexture_create(mode.width, mode.height, sfFalse);
    sfSprite* light_sprite = sfSprite_create();
    sfSprite_setTexture(light_sprite, sfRenderTexture_getTexture(light_texture), sfTrue);

    FpsCounter* fps = FpsCounter_create();

    sfClock* clock = sfClock_create();
    float time_step = 1.0 / 60.0;
    float elapsed_time = 0.0;

    ComponentData* components = ComponentData_create();
    ColliderGrid* grid = ColliderGrid_create();
    int camera = create_camera(components, mode);

    float ambient_light = 0.5;
    create_level(components, grid, time(NULL));
    init_grid(components, grid);
    init_waypoints(components, grid);

    while (sfRenderWindow_isOpen(window))
    {
        float delta_time = sfTime_asSeconds(sfClock_restart(clock));

        sfEvent event;
        while (sfRenderWindow_pollEvent(window, &event))
        {
            if (event.type == sfEvtLostFocus) {
                focus = false;
            }

            if (event.type == sfEvtGainedFocus) {
                focus = true;
                sfClock_restart(clock);
            }

            if (event.type == sfEvtClosed) {
                sfRenderWindow_close(window);
            }

            if (event.type == sfEvtKeyPressed && event.key.code == sfKeyEscape) {
                sfRenderWindow_close(window);
            }
        }

        if (focus) {
            while (elapsed_time > time_step) {
                elapsed_time -= time_step;

                sfJoystick_update();
                input(components, window, camera);

                update(components, time_step, grid);
                collide(components, grid);
                update_waypoints(components, grid);

                update_players(components, grid, time_step);
                update_weapons(components, time_step);
                update_enemies(components, grid);

                update_particles(components, time_step);
                update_lights(components, time_step);
                update_camera(components, camera, time_step);

                draw_lights(components, grid, light_texture, camera, ambient_light);
            }

            elapsed_time += delta_time;
        }

        sfRenderWindow_clear(window, sfColor_fromRGB(100, 100, 100));

        draw(components, window, camera, textures);
        sfRenderWindow_drawSprite(window, light_sprite, &state);
        draw_roofs(components, window, camera, textures);
        draw_outlines(components, window, camera);
        draw_hud(components, window, camera);

        // debug_draw(components, window, camera);
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
