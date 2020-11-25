#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>

#include <SFML/System/Vector2.h>
#include <SFML/Window/Keyboard.h>

#include "util.h"
#include "component.h"
#include "camera.h"
#include "collider.h"
#include "light.h"
#include "util.h"


void shoot(Component* component, ColliderGrid* grid, int i, float delta_time) {
    PlayerComponent* player = component->player[i];

    if (player->cooldown <= 0.0) {
        player->cooldown = 1.0 / player->fire_rate;

        float angle = component->coordinate[i]->angle + float_rand(-0.5 * player->recoil, 0.5 * player->recoil);
        sfVector2f r = polar_to_cartesian(0.6, angle);

        sfVector2f pos = sum(component->coordinate[i]->position, r);

        HitInfo info = raycast(component, grid, pos, r, 20.0);

        if (component->enemy[info.object]) {
            component->enemy[info.object]->health -= 10;
            component->physics[info.object]->velocity = polar_to_cartesian(2.0, angle);

            component->particle[info.object]->enabled = true;
        }

        player->recoil = fmin(0.5 * M_PI, player->recoil + delta_time * 15.0);
    }
}


void input(Component* component, sfRenderWindow* window, ColliderGrid* grid, Camera* camera, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->player[i]) continue;

        sfVector2f v = { 0.0, 0.0 };

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

        player->cooldown -= delta_time;
        player->recoil = fmax(0.1 * norm(phys->velocity), player->recoil - delta_time * player->recoil_reduction);

        if (sfMouse_isButtonPressed(sfMouseLeft)) {
            shoot(component, grid, i, delta_time);
        }

        camera->position = sum(camera->position, mult(10.0 * delta_time, diff(coord->position, camera->position)));
    }
}


void create_player(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->image[i] = ImageComponent_create("player", 1.0);
    component->physics[i] = PhysicsComponent_create(1.0, 0.0, 0.0, 10.0);
    component->physics[i]->max_speed = 5.0;
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
    component->player[i] = PlayerComponent_create();
    component->light[i] = LightComponent_create(10.0, 1.0, 101, 0.2);
    component->particle[i] = ParticleComponent_create(1.0, 0.1, 5.0, sfColor_fromRGBA(255, 255, 0, 128));
}


void draw_player(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->player[i]) continue;

        sfVector2f r = polar_to_cartesian(1.0, component->coordinate[i]->angle + 0.5 * component->player[i]->recoil);

        sfVector2f start = sum(component->coordinate[i]->position, mult(0.5, r));
        sfVector2f end = sum(component->coordinate[i]->position, mult(3.0, r));
        draw_line(window, camera, NULL, start, end, 0.05, sfWhite);

        r = polar_to_cartesian(1.0, component->coordinate[i]->angle - 0.5 * component->player[i]->recoil);

        start = sum(component->coordinate[i]->position, mult(0.5, r));
        end = sum(component->coordinate[i]->position, mult(3.0, r));
        draw_line(window, camera, NULL, start, end, 0.05, sfWhite);
    }
}
