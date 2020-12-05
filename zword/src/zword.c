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

    Component* component = Component_create();

    Camera* camera = Camera_create(mode);

    ColliderGrid* grid = ColliderGrid_create();

    create_level(component);

    for (int i = 0; i < component->entities; i++) {
        update_grid(component, grid, i);
    }

    init_waypoints(component, grid);

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

                input(component, window, grid, camera, time_step);

                //update_enemies(component);

                update(component, time_step, grid);
                collide(component, grid);

                update_particles(component, time_step);

                update_lights(component, time_step);

                update_waypoints(component, grid);
            }
        }

        sfRenderWindow_clear(window, sfColor_fromRGB(100, 100, 100));

        draw_grid(window, camera);

        debug_draw(component, window, camera);

        draw_particles(component, window, camera);

        draw_lights(component, grid, window, texture, camera, ambient_light);
        sfRenderStates state = { sfBlendMultiply, sfTransform_Identity, NULL, NULL };
        sfSprite_setTexture(sprite, sfRenderTexture_getTexture(texture), sfTrue);
        sfRenderWindow_drawSprite(window, sprite, &state);

        draw_player(component, window, camera);

        //draw_waypoints(component, window, camera);

        draw_enemies(component, window, camera);

        float delta_time = sfTime_asSeconds(sfClock_restart(clock));
        elapsed_time += delta_time;

        draw_fps(window, fps, delta_time);

        sfRenderWindow_display(window);
    }

    for (int i = 0; i < component->entities; i++) {
        destroy_entity(component, i);
    }

    sfRenderWindow_destroy(window);

    return 0;
}
