#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include <SFML/System/Vector2.h>
#include <SFML/Window/Keyboard.h>

#include "util.h"
#include "component.h"
#include "camera.h"


void shoot(Component* component, int i) {
    int j = component->entities;
    component->entities++;

    float angle = component->coordinate[i]->angle;
    sfVector2f r = polar_to_cartesian(0.6, angle);

    sfVector2f pos = sum(component->coordinate[i]->position, r);

    component->coordinate[j] = CoordinateComponent_create(pos, 0.0);
    component->circle_collider[j] = CircleColliderComponent_create(0.1);
    component->physics[j] = PhysicsComponent_create(1.0, 0.0, 0.5, 0.5);

    component->physics[j]->velocity = mult(20.0, r);
}


void input(Component* component, sfRenderWindow* window, Camera* camera, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->player[i]) continue;

        sfVector2f v = { 0, 0 };

        if (sfKeyboard_isKeyPressed(sfKeyA)) {
            v.x -= 1;
        }

        if (sfKeyboard_isKeyPressed(sfKeyD)) {
            v.x += 1;
        }

        if (sfKeyboard_isKeyPressed(sfKeyW)) {
            v.y += 1;
        }

        if (sfKeyboard_isKeyPressed(sfKeyS)) {
            v.y -= 1;
        }

        CoordinateComponent* coord = component->coordinate[i];
        PhysicsComponent* phys = component->physics[i];
        PlayerComponent* player = component->player[i];

        phys->acceleration = sum(phys->acceleration, mult(player->acceleration, normalized(v)));

        sfVector2f mouse = screen_to_world(sfMouse_getPosition((sfWindow*) window), camera);
        sfVector2f rel_mouse = diff(mouse, coord->position);

        coord->angle = atan2(rel_mouse.y, rel_mouse.x);

        if (player->cooldown < 0.0 && sfMouse_isButtonPressed(sfMouseLeft)) {
            shoot(component, i);
            player->cooldown = 0.25;
        }

        player->cooldown -= delta_time;

        camera->position = sum(camera->position, mult(10.0 * delta_time, diff(coord->position, camera->position)));
    }
}


void create_player(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->image[i] = ImageComponent_create("player", 1.0);
    component->physics[i] = PhysicsComponent_create(1.0, 0.0, 0.0, 2.0);
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
    component->player[i] = PlayerComponent_create();
}