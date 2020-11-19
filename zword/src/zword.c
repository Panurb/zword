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
#define MAX_ENTITIES 100


int main() {
    sfVideoMode mode = { 1280, 720, 32 };
    sfRenderWindow* window = sfRenderWindow_create(mode, "zword", sfResize | sfClose, NULL);
    if (!window) {
        return 1;
    }

    //sfWindow_setVerticalSyncEnabled(window, true);
    //sfWindow_setFramerateLimit(window, 60);

    sfClock* clock = sfClock_create();
    sfEvent event;

    float frame_times[10] = { 0.0 };

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
    
    sfVector2f cam_pos = { 0, 0 };
    Camera camera = { cam_pos, 128.0, mode.width, mode.height };

    create_wall(component, 3.0, 0.0, 0.5, 6.0, 0.0);
    create_wall(component, -3.0, 0.0, 0.5, 6.0, 0.0);
    create_wall(component, 0.0, -3.0, 6.0, 0.5, 0.0);
    create_wall(component, 0.0, 3.0, 6.0, 0.5, 0.0);

    create_prop(component, 0.0, 0.0, 1.0, 1.0, 0.4);
    create_prop(component, 0.0, 1.0, 1.0, 1.0, 0.4);
    create_prop(component, 0.0, -1.0, 1.0, 1.0, 0.4);

    create_player(component, 2.0, 0.0);

    int i = 0;
    while (sfRenderWindow_isOpen(window))
    {
        while (sfRenderWindow_pollEvent(window, &event))
        {
            if (event.type == sfEvtClosed) {
                sfRenderWindow_close(window);
            }
        }

        sfVector2f mouse = screen_to_world(sfMouse_getPosition(window), &camera);
        input(component, mouse);

        frame_times[i] = sfTime_asSeconds(sfClock_restart(clock));
        elapsed_time += frame_times[i];
        i = (i + 1) % 10;

        while (elapsed_time > delta_time) {
            elapsed_time -= delta_time;

            update(component, delta_time);
            collide(component);
        }

        sfRenderWindow_clear(window, sfBlack);

        //draw(component, window, &camera);
        debug_draw(component, window, &camera);

        float fps = 1.0 / mean(frame_times, 10);

        char buffer[10];
        snprintf(buffer, 20, "%f", fps);
        sfText_setString(text, buffer);

        sfRenderWindow_drawText(window, text, NULL);

        sfRenderWindow_display(window);
    }

    sfRenderWindow_destroy(window);

    return 0;
}
