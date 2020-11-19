#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include <SFML/System/Vector2.h>
#include <SFML/Window/Keyboard.h>

#include "util.h"
#include "component.h"


void shoot(Component* component, int i) {
    int j = component->entities;
    component->entities++;

    float angle = component->coordinate[i]->angle;
    sfVector2f r = polar_to_cartesian(0.6, angle);

    sfVector2f pos = sum(component->coordinate[i]->position, r);

    component->coordinate[j] = CoordinateComponent_create(pos.x, pos.y, 0.0);
    component->circle_collider[j] = CircleColliderComponent_create(0.1);
    component->physics[j] = PhysicsComponent_create(1.0, 0.0, 0.5);

    component->physics[j]->velocity = mult(10.0, r);
}


void input(Component* component, sfVector2f mouse) {
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

        float n = norm(v);

        if (n > 0) {
            component->physics[i]->acceleration.x += v.x * component->player[i]->acceleration / n;
            component->physics[i]->acceleration.y += v.y * component->player[i]->acceleration / n;
        }

        mouse.x -= component->coordinate[i]->position.x;
        mouse.y -= component->coordinate[i]->position.y;

        component->coordinate[i]->angle = atan2(mouse.y, mouse.x);

        if (component->player[i]->cooldown < 0.0 && sfMouse_isButtonPressed(sfMouseLeft)) {
            shoot(component, i);
            component->player[i]->cooldown = 0.5;
        }

        component->player[i]->cooldown -= 1.0 / 60.0;
    }
}


void create_player(Component* component, float x, float y) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(x, y, 0.0);
    component->image[i] = ImageComponent_create("player", 1.0);
    component->physics[i] = PhysicsComponent_create(1.0, 0.0, 0.0);
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
    component->player[i] = PlayerComponent_create();
}
