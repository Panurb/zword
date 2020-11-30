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
#include "raycast.h"
#include "util.h"


void shoot(Component* component, ColliderGrid* grid, int i, float delta_time) {
    PlayerComponent* player = component->player[i];

    int item = player->inventory[player->item];
    WeaponComponent* weapon = component->weapon[item];

    if (weapon->cooldown <= 0.0) {
        weapon->cooldown = 1.0 / weapon->fire_rate;

        float angle = float_rand(-0.5 * weapon->recoil, 0.5 * weapon->recoil);
        sfVector2f r = polar_to_cartesian(0.6, component->coordinate[i]->angle + angle);

        sfVector2f pos = sum(component->coordinate[i]->position, r);

        HitInfo info = raycast(component, grid, pos, r, 20.0);

        if (component->enemy[info.object]) {
            component->enemy[info.object]->health -= 10;
            component->physics[info.object]->velocity = polar_to_cartesian(2.0, component->coordinate[i]->angle + angle);

            component->particle[info.object]->enabled = true;
        }

        weapon->recoil = fmin(0.5 * M_PI, weapon->recoil + delta_time * weapon->recoil_up);
        component->particle[item]->angle = angle;
        component->particle[item]->max_time = dist(pos, info.position) / component->particle[item]->speed;
        component->particle[item]->enabled = true;
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

        v = normalized(v);

        CoordinateComponent* coord = component->coordinate[i];
        PlayerComponent* player = component->player[i];

        if (player->vehicle == -1) {
            PhysicsComponent* phys = component->physics[i];

            phys->acceleration = sum(phys->acceleration, mult(player->acceleration, v));

            sfVector2f mouse = screen_to_world(sfMouse_getPosition((sfWindow*) window), camera);
            sfVector2f rel_mouse = diff(mouse, coord->position);

            coord->angle = atan2(rel_mouse.y, rel_mouse.x);

            int item = player->inventory[player->item];

            if (item != -1) {
                WeaponComponent* weapon = component->weapon[item];
                weapon->cooldown -= delta_time;
                weapon->recoil = fmax(0.1 * norm(phys->velocity), weapon->recoil - delta_time * weapon->recoil_down);

                if (sfMouse_isButtonPressed(sfMouseLeft)) {
                    shoot(component, grid, i, delta_time);
                }
            }

            if (sfMouse_isButtonPressed(sfMouseRight)) {
                for (int j = 0; j < component->entities; j++) {
                    if (component->weapon[j]) {
                        if (dist(coord->position, component->coordinate[j]->position) < 1.0) {
                            int k = find(-1, player->inventory, player->inventory_size);
                            if (k != -1) {
                                player->inventory[k] = j;
                                component->coordinate[j]->parent = i;
                                component->coordinate[j]->position = (sfVector2f) { 0.0, 0.0 };
                                component->coordinate[j]->angle = 0.0;
                                component->rectangle_collider[j]->enabled = false;
                            }
                            break;
                        }
                    }
                }
            }

            if (sfKeyboard_isKeyPressed(sfKeyF)) {
                for (int j = 0; j < component->entities; j++) {
                    if (component->vehicle[j]) {
                        if (dist(coord->position, component->coordinate[j]->position) < 3.0) {
                            player->vehicle = j;
                            component->vehicle[j]->driver = i;
                            component->circle_collider[i]->enabled = false;
                            component->light[i]->enabled = false;
                            break;
                        }
                    }
                }
            }
        } else {
            VehicleComponent* vehicle = component->vehicle[player->vehicle];

            PhysicsComponent* phys = component->physics[player->vehicle];

            coord->position = component->coordinate[player->vehicle]->position;

            sfVector2f r = polar_to_cartesian(1.0, component->coordinate[player->vehicle]->angle);
            sfVector2f at = mult(vehicle->acceleration * v.y, r);
            phys->acceleration = sum(phys->acceleration, at);   

            if (norm(phys->velocity) > 1.2) {
                phys->angular_acceleration -= sign(v.y) * vehicle->turning * v.x;
            }

            sfVector2f v_new = rotate(phys->velocity, phys->angular_velocity * delta_time);

            phys->acceleration = sum(phys->acceleration, mult(1.0 / delta_time, diff(v_new, phys->velocity)));

            if (sfKeyboard_isKeyPressed(sfKeyE)) {
                player->vehicle = -1;
                vehicle->driver = -1;
                component->circle_collider[i]->enabled = true;
                component->light[i]->enabled = true;
            }
        }

        camera->position = sum(camera->position, mult(10.0 * delta_time, diff(coord->position, camera->position)));
        if (player->vehicle != -1) {
            camera->zoom += 10.0 * delta_time * (25.0 - camera->zoom);
        } else {
            camera->zoom += 10.0 * delta_time * (40.0 - camera->zoom);
        }
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
    component->light[i] = LightComponent_create(10.0, 1.0, 101, sfWhite, 0.25);
    //component->particle[i] = ParticleComponent_create(1.0, 0.1, 5.0, sfColor_fromRGBA(255, 255, 0, 128));
}


void draw_player(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        PlayerComponent* player = component->player[i];

        if (!player) continue;

        if (player->vehicle == -1 && player->inventory[player->item] != -1) {
            WeaponComponent* weapon = component->weapon[player->inventory[player->item]];
            if (weapon) {
                sfVector2f r = polar_to_cartesian(1.0, component->coordinate[i]->angle + 0.5 * weapon->recoil);

                sfVector2f start = sum(component->coordinate[i]->position, mult(0.5, r));
                sfVector2f end = sum(component->coordinate[i]->position, mult(3.0, r));
                draw_line(window, camera, NULL, start, end, 0.05, sfWhite);

                r = polar_to_cartesian(1.0, component->coordinate[i]->angle - 0.5 * weapon->recoil);

                start = sum(component->coordinate[i]->position, mult(0.5, r));
                end = sum(component->coordinate[i]->position, mult(3.0, r));
                draw_line(window, camera, NULL, start, end, 0.05, sfWhite);
            }
        }

        if (sfKeyboard_isKeyPressed(sfKeySpace)) {
            float angle = get_angle(component, i);
            player->item = floor(player->inventory_size * mod(angle, 2 * M_PI) / (2 * M_PI));
            for (int i = 0; i < player->inventory_size; i++) {

                if (player->inventory[i] != -1) {

                }
            }
        }
    }
}
