#include <stdlib.h>
#include <math.h>

#include <SFML/System/Vector2.h>
#include <SFML/Window/Keyboard.h>

#include "util.h"
#include "physics.h"
#include "image.h"
#include "collider.h"


void input(Component* component, sfVector2f mouse) {
    for (int i = 0; i <= component->entities; i++) {
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
            component->physics[i]->acceleration.x += v.x * component->player[i]->speed / n;
            component->physics[i]->acceleration.y += v.y * component->player[i]->speed / n;
        }

        mouse.x -= component->coordinate[i]->position.x;
        mouse.y -= component->coordinate[i]->position.y;

        component->coordinate[i]->angle = atan2(mouse.y, mouse.x);
    }
}


void create_player(Component* component) {
    int i = component->entities;
    component->entities++;

    CoordinateComponent* coord = malloc(sizeof(CoordinateComponent));
    sfVector2f pos = { 0, 0 };
    coord->position = pos;
    coord->angle = 0.0;
    component->coordinate[i] = coord;

    ImageComponent* image = malloc(sizeof(ImageComponent));
    sfVector2f scale = { 1, 1 };
    sfSprite* sprite = load_sprite("player", scale);
    image->scale = scale;
    image->sprite = sprite;
    component->image[i] = image;

    PhysicsComponent* phys = malloc(sizeof(PhysicsComponent));
    sfVector2f vel = { 0, 0 };
    sfVector2f acc = { 0, 0 };
    phys->velocity = vel;
    phys->acceleration = acc;
    phys->angular_velocity = 0.0;
    phys->friction = 0.01;
    component->physics[i] = phys;

    CircleColliderComponent* col = malloc(sizeof(CircleColliderComponent));
    col->position = pos;
    col->radius = 0.5;
    col->shape = sfCircleShape_create();
    component->circle_collider[i] = col;

    PlayerComponent* player = malloc(sizeof(PlayerComponent));
    player->health = 100;
    player->speed = 0.02;
    component->player[i] = player;
}
