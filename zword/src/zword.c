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
#include "light.h"
#include "grid.h"
#include "enemy.h"
#include "particle.h"


// remove from here
#define MAX_ENTITIES 1000
#define FRAME_WINDOW 100


int main() {
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

    float frame_times[FRAME_WINDOW] = { 0.0 };
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
            while (elapsed_time > delta_time) {
                elapsed_time -= delta_time;

                input(component, window, grid, camera, delta_time);

                update_enemy(component, grid);

                update(component, delta_time, grid);
                collide(component, grid);

                update_particles(component, delta_time);

                update_lights(component, delta_time);
            }
        }

        sfRenderWindow_clear(window, sfColor_fromRGB(100, 100, 100));

        draw_grid(window, camera);

        debug_draw(component, window, camera);
        //draw(component, window, camera);

        draw_particles(component, window, camera);

        draw_lights(component, grid, window, texture, camera, ambient_light);
        sfRenderStates state = { sfBlendMultiply, sfTransform_Identity, NULL, NULL };
        sfSprite_setTexture(sprite, sfRenderTexture_getTexture(texture), sfTrue);
        sfRenderWindow_drawSprite(window, sprite, &state);

        draw_player(component, window, camera);

        frame_avg -= frame_times[i] / FRAME_WINDOW;
        frame_times[i] = sfTime_asSeconds(sfClock_restart(clock));
        frame_avg += frame_times[i] / FRAME_WINDOW;

        elapsed_time += frame_times[i];
        i = (i + 1) % FRAME_WINDOW;

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
