#include <stdio.h>
#include <stdbool.h>

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


// remove from here
#define MAX_ENTITIES 1000


int main() {
    sfVideoMode mode = { 1280, 720, 32 };
    sfRenderWindow* window = sfRenderWindow_create(mode, "zword", sfResize | sfClose, NULL);
    if (!window) {
        return 1;
    }

    bool focus = true;

    //sfWindow_setVerticalSyncEnabled(window, true);
    //sfWindow_setFramerateLimit(window, 60);

    sfClock* clock = sfClock_create();
    sfEvent event;

    float frame_times[1000] = { 0.0 };
    float frame_avg = 0.0;

    float delta_time = 1.0 / 60.0;
    float elapsed_time = 0.0;

    sfFont* font = sfFont_createFromFile("data/Helvetica.ttf");
    if (!font) {
        printf("Font not found!");
        return 1;
    }

    sfText* text = sfText_create();
    sfText_setFont(text, font);
    sfText_setCharacterSize(text, 20);
    sfText_setColor(text, sfWhite);

    Component comp = { .entities=0 };
    Component* component = &comp;
    for (int i = 0; i < MAX_ENTITIES; i++) {
        component->coordinate[i] = NULL;
        component->image[i] = NULL;
        component->physics[i] = NULL;
        component->circle_collider[i] = NULL;
        component->rectangle_collider[i] = NULL;
        component->player[i] = NULL;
    }
    
    Camera* camera = Camera_create(mode);

    ColliderGrid* grid = ColliderGrid_create();

    create_level(component);

    for (int i = 0; i < component->entities; i++) {
        if (component->circle_collider[i] && !component->circle_collider[i]->enabled) {
            continue;
        }

        update_grid(component, grid, i);
    }

    int i = 0;
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
            frame_avg -= frame_times[i] / 1000.0;
            frame_times[i] = sfTime_asSeconds(sfClock_restart(clock));
            frame_avg += frame_times[i] / 1000.0;

            elapsed_time += frame_times[i];
            i = (i + 1) % 1000;

            while (elapsed_time > delta_time) {
                elapsed_time -= delta_time;

                input(component, window, grid, camera, delta_time);

                update(component, delta_time, grid);
                collide(component, grid);
            }
        }

        sfRenderWindow_clear(window, sfBlack);

        draw_grid(window, camera);

        //draw(component, window, camera);
        player_debug_draw(component, grid, window, camera);
        debug_draw(component, grid, window, camera);

        char buffer[20];
        snprintf(buffer, 20, "%.0f", 1.0 / frame_avg);
        sfText_setString(text, buffer);

        sfRenderWindow_drawText(window, text, NULL);

        sfRenderWindow_display(window);
    }

    for (int i = 0; i < component->entities; i++) {
        destroy_entity(component, i);
    }

    sfRenderWindow_destroy(window);

    return 0;
}
