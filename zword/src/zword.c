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


int main() {
    srand(time(NULL));

    sfVideoMode mode = { 1280, 720, 32 };
    sfContext* context = sfContext_create();
    sfContextSettings settings = sfContext_getSettings(context);
    settings.antialiasingLevel = 8;
    sfRenderWindow* window = sfRenderWindow_create(mode, "zword", sfClose, &settings);
    if (!window) {
        return 1;
    }
    sfRenderWindow_setKeyRepeatEnabled(window, sfFalse);

    sfTexture* textures[100];

    load_textures(textures);

    sfRenderTexture* texture = sfRenderTexture_create(mode.width, mode.height, sfFalse);
    sfSprite* sprite = sfSprite_create();

    float ambient_light = 0.5;

    bool focus = true;

    //sfWindow_setVerticalSyncEnabled(window, true);
    //sfWindow_setFramerateLimit(window, 60);

    sfClock* clock = sfClock_create();
    sfEvent event;

    FpsCounter* fps = FpsCounter_create();

    float time_step = 1.0 / 60.0;
    float elapsed_time = 0.0;

    ComponentData* components = ComponentData_create();

    Camera* camera = Camera_create(mode);

    ColliderGrid* grid = ColliderGrid_create();

    create_level(components, 64.0, 64.0);

    init_grid(components, grid);
    init_waypoints(components, grid);

    while (sfRenderWindow_isOpen(window))
    {
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
        }

        if (focus) {
            while (elapsed_time > time_step) {
                elapsed_time -= time_step;

                input(components);

                update_players(components, grid, window, camera, time_step);

                update_enemies(components, grid);

                update(components, time_step, grid);
                collide(components, grid);

                update_particles(components, time_step);

                update_lights(components, time_step);

                update_waypoints(components, grid);

                update_camera(components, camera, time_step);
            }
        }

        sfRenderWindow_clear(window, sfColor_fromRGB(100, 100, 100));

        //draw_grid(window, camera);

        //debug_draw(component, window, camera);

        draw(components, window, camera, textures);

        draw_particles(components, window, camera);

        draw_lights(components, grid, window, textures, texture, camera, ambient_light);
        sfRenderStates state = { sfBlendMultiply, sfTransform_Identity, NULL, NULL };
        sfSprite_setTexture(sprite, sfRenderTexture_getTexture(texture), sfTrue);
        sfRenderWindow_drawSprite(window, sprite, &state);

        draw_players(components, window, camera);

        draw_waypoints(components, window, camera);

        //draw_enemies(components, window, camera);

        float delta_time = sfTime_asSeconds(sfClock_restart(clock));
        elapsed_time += delta_time;

        draw_fps(window, fps, delta_time);

        sfRenderWindow_display(window);
    }

    for (int i = 0; i < components->entities; i++) {
        destroy_entity(components, i);
    }

    sfRenderWindow_destroy(window);

    return 0;
}
