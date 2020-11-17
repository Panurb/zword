#include <stdio.h>


#include <SFML/Audio.h>
#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>
#include <SFML/Window/Keyboard.h>

#include "camera.h"
#include "image.h"
#include "player.h"
#include "collider.h"
#include "physics.h"
#include "collider.h"
#include "component.h"
#include "level.h"


// remove from here
#define MAX_ENTITIES 100


int main()
{
    sfVideoMode mode = { 1280, 720, 32 };
    sfRenderWindow* window;
    sfEvent event;

    float delta_time = 1.0 / 60.0;

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

    create_player(component);
    create_wall(component);

    window = sfRenderWindow_create(mode, "SFML window", sfResize | sfClose, NULL);
    if (!window)
        return 1;

    while (sfRenderWindow_isOpen(window))
    {
        while (sfRenderWindow_pollEvent(window, &event))
        {
            if (event.type == sfEvtClosed)
                sfRenderWindow_close(window);
        }

        sfVector2f mouse = screen_to_world(sfMouse_getPosition(window), &camera);
        input(component, mouse);

        update(component, delta_time);

        collide(component);

        sfRenderWindow_clear(window, sfBlack);

        //draw(component, window, &camera);
        debug_draw(component, window, &camera);

        sfRenderWindow_display(window);
    }

    sfRenderWindow_destroy(window);

    return 0;
}
