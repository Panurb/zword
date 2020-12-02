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
#include "weapon.h"


void pick_up_item(Component* component, int i) {
    PlayerComponent* player = component->player[i];

    for (int j = 0; j < component->entities; j++) {
        if (component->item[j] && component->coordinate[j]->parent == -1) {
            float d = dist(component->coordinate[i]->position, component->coordinate[j]->position);
            if (d < 1.0) {
                int k = find(-1, player->inventory, player->inventory_size);
                if (k != -1) {
                    player->inventory[k] = j;
                    component->coordinate[j]->parent = i;
                    component->coordinate[j]->position = (sfVector2f) { 0.0, 0.0 };
                    component->coordinate[j]->angle = 0.0;
                    component->rectangle_collider[j]->enabled = false;

                    if (component->light[j]) {
                        component->light[j]->enabled = true;
                    }
                }
                break;
            }
        }
    }
}


void drop_item(Component* component, int i) {
    PlayerComponent* player = component->player[i];

    int j = player->inventory[player->item];
    if (j != -1) {
        player->inventory[player->item] = -1;
        component->coordinate[j]->parent = -1;
        sfVector2f r = polar_to_cartesian(1.0, get_angle(component, i));
        component->coordinate[j]->position = sum(get_position(component, i), r);
        component->rectangle_collider[j]->enabled = true;
        component->physics[j]->velocity = mult(7.0, r);
        component->physics[j]->angular_velocity = 3.0;

        if (component->light[j]) {
            component->light[j]->enabled = false;
        }
    }
}


void enter_vehicle(Component* component, int i) {
    for (int j = 0; j < component->entities; j++) {
        if (component->vehicle[j]) {
            float d = dist(get_position(component, i), get_position(component, j));
            if (d < 3.0) {
                int item = component->player[i]->inventory[component->player[i]->item];
                if (component->light[item]) {
                    component->light[item]->enabled = false;
                }

                component->player[i]->vehicle = j;
                component->vehicle[j]->driver = i;
                component->circle_collider[i]->enabled = false;
                break;
            }
        }
    }
}


void exit_vehicle(Component* component, int i) {
    int j = component->player[i]->vehicle;
    component->player[i]->vehicle = -1;
    component->vehicle[j]->driver = -1;
    component->circle_collider[i]->enabled = true;
}


void drive_vehicle(Component* component, int i, sfVector2f v, float delta_time) {
    int j = component->player[i]->vehicle;

    VehicleComponent* vehicle = component->vehicle[j];

    PhysicsComponent* phys = component->physics[j];

    component->coordinate[i]->position = component->coordinate[j]->position;

    sfVector2f r = polar_to_cartesian(1.0, component->coordinate[j]->angle);
    sfVector2f at = mult(vehicle->acceleration * v.y, r);
    phys->acceleration = sum(phys->acceleration, at);   

    if (norm(phys->velocity) > 1.2) {
        phys->angular_acceleration -= sign(v.y) * vehicle->turning * v.x;
    }

    sfVector2f v_new = rotate(phys->velocity, phys->angular_velocity * delta_time);

    phys->acceleration = sum(phys->acceleration, mult(1.0 / delta_time, diff(v_new, phys->velocity)));
}


sfVector2f left_stick() {
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

    return normalized(v);
}


sfVector2f right_stick(Component* component, sfRenderWindow* window, Camera* camera, int i) {
    sfVector2f mouse = screen_to_world(sfMouse_getPosition((sfWindow*) window), camera);
    sfVector2f rel_mouse = diff(mouse, get_position(component, i));

    return normalized(rel_mouse);
}


int get_inventory_slot(Component* component, sfRenderWindow* window, Camera* camera, int i) {
    PlayerComponent* player = component->player[i];

    sfVector2f rs = right_stick(component, window, camera, i);
    float angle = polar_angle(rs);

    return floor(player->inventory_size * mod(angle + 0.25 * M_PI, 2 * M_PI) / (2 * M_PI));
}


void input(Component* component, sfRenderWindow* window, ColliderGrid* grid, Camera* camera, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->player[i]) continue;

        sfVector2f v = left_stick();

        CoordinateComponent* coord = component->coordinate[i];
        PlayerComponent* player = component->player[i];

        if (player->vehicle == -1) {
            PhysicsComponent* phys = component->physics[i];

            phys->acceleration = sum(phys->acceleration, mult(player->acceleration, v));

            int item = player->inventory[player->item];

            int slot = get_inventory_slot(component, window, camera, i);

            if (sfKeyboard_isKeyPressed(sfKeySpace)) {
                if (sfMouse_isButtonPressed(sfMouseLeft)) {
                    if (player->grabbed_item == -1) {
                        player->grabbed_item = slot;
                    }
                } else {
                    if (player->grabbed_item != -1) {
                        if (player->inventory[slot] == -1) {
                            player->inventory[slot] = player->inventory[player->grabbed_item];
                            player->inventory[player->grabbed_item] = -1;
                        }
                        player->grabbed_item = -1;
                    } else {
                        if (component->light[item]) {
                            component->light[item]->enabled = false;
                        }

                        player->item = slot;

                        if (sfMouse_isButtonPressed(sfMouseRight)) {
                            drop_item(component, i);
                        }
                    }
                }
            } else {
                if (sfMouse_isButtonPressed(sfMouseRight)) {
                    pick_up_item(component, i);
                }

                coord->angle = polar_angle(right_stick(component, window, camera, i));

                item = player->inventory[player->item];

                if (item != -1) {
                    if (component->weapon[item]) {
                        WeaponComponent* weapon = component->weapon[item];

                        weapon->cooldown -= delta_time;
                        weapon->recoil = fmax(0.1 * norm(phys->velocity), weapon->recoil - delta_time * weapon->recoil_down);

                        if (sfMouse_isButtonPressed(sfMouseLeft)) {
                            shoot(component, grid, i, delta_time);
                        }
                    }
                    
                    if (component->light[item]) {
                        component->light[item]->enabled = true;
                    }
                }
            }

            if (sfKeyboard_isKeyPressed(sfKeyF)) {
                enter_vehicle(component, i);
            }
        } else {
            drive_vehicle(component, i, v, delta_time);

            if (sfKeyboard_isKeyPressed(sfKeyE)) {
                exit_vehicle(component, i);
            }
        }

        camera->position = sum(camera->position, mult(10.0 * delta_time, diff(coord->position, camera->position)));
        if (player->vehicle != -1) {
            camera->zoom += 10.0 * delta_time * (25.0 - camera->zoom);
        } else {
            camera->zoom += 10.0 * delta_time * (25.0 - camera->zoom);
        }
    }
}


void create_player(Component* component, sfVector2f pos) {
    int i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->image[i] = ImageComponent_create("player", 1.0);
    component->physics[i] = PhysicsComponent_create(1.0, 0.0, 0.0, 10.0, 0.0);
    component->physics[i]->max_speed = 5.0;
    component->circle_collider[i] = CircleColliderComponent_create(0.5);
    component->player[i] = PlayerComponent_create();
    component->particle[i] = ParticleComponent_create(0.0, 2 * M_PI, 0.5, 0.0, 5.0, 10.0, sfColor_fromRGB(200, 0, 0), sfColor_fromRGB(255, 0, 0));
    component->waypoint[i] = WaypointComponent_create();
}


void draw_player(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        PlayerComponent* player = component->player[i];

        if (!player) continue;

        if (sfKeyboard_isKeyPressed(sfKeySpace)) {
            sfVector2f pos = get_position(component, i);

            sfConvexShape* shape = sfConvexShape_create();
            sfColor color = sfWhite;
            sfConvexShape_setPointCount(shape, 4);

            sfRectangleShape* line = sfRectangleShape_create();

            float gap = 0.1;
            float slice = (2 * M_PI / player->inventory_size);
            int slot = get_inventory_slot(component, window, camera, i);

            for (int j = 0; j < player->inventory_size; j++) {
                float offset = 0.0;
                color.a = 255;

                if (j == slot) {
                    if (player->grabbed_item == -1) {
                        offset = 0.1;
                    } else if (player->inventory[j] == -1) {
                        offset = 0.1;
                    }
                } else {
                    if (j == player->grabbed_item) {
                        color.a = 64;
                        draw_slice_outline(window, camera, line, pos, 1.0 + offset, 1.5 + offset, j * slice, slice - gap);
                    }
                }
                
                if (player->inventory[j] == -1) {
                    color.a = 64;
                }

                sfConvexShape_setFillColor(shape, color);

                draw_slice(window, camera, shape, pos, 1.0 + offset, 1.5 + offset, j * slice, slice - gap);
            
                if (player->inventory[j] != -1) {
                    // draw item
                }
            }
        } else {
            if (player->vehicle == -1 && player->inventory[player->item] != -1) {
                WeaponComponent* weapon = component->weapon[player->inventory[player->item]];
                if (weapon) {
                    sfConvexShape* shape = sfConvexShape_create();
                    sfConvexShape_setPointCount(shape, 20);

                    sfConvexShape_setOutlineColor(shape, sfWhite);
                    sfConvexShape_setOutlineThickness(shape, 0.02 * camera->zoom);

                    sfColor color = sfWhite;
                    color.a = 0;
                    sfConvexShape_setFillColor(shape, color);

                    float spread = fmax(0.01, weapon->recoil);
                    draw_cone(window, camera, shape, 20, get_position(component, i), 3.0, get_angle(component, i), spread);
                }
            }
        }
    }
}
