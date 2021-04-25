#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <SFML/System/Vector2.h>
#include <SFML/Window/Keyboard.h>

#include "util.h"
#include "component.h"
#include "camera.h"
#include "collider.h"
#include "raycast.h"
#include "util.h"
#include "weapon.h"
#include "vehicle.h"
#include "item.h"
#include "image.h"
#include "input.h"
#include "particle.h"
#include "enemy.h"


void create_player(ComponentData* components, sfVector2f pos, int joystick) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    ImageComponent_add(components, i, "player", 1.0, 1.0, 5);
    PhysicsComponent_add(components, i, 1.0, 0.0, 0.0, 10.0, 0.0)->max_speed = 5.0;
    ColliderComponent_add_circle(components, i, 0.5, PLAYERS);
    PlayerComponent* player = PlayerComponent_add(components, i, joystick);
    ParticleComponent_add_blood(components, i);
    WaypointComponent_add(components, i)->range = 12.0;
    HealthComponent_add(components, i, 100);
    SoundComponent_add(components, i, "squish");

    for (AmmoType type = AMMO_PISTOL; type <= AMMO_SHOTGUN; type++) {
        int j = create_ammo(components, zeros(), type);
        player->ammo[type] = j;
        CoordinateComponent_get(components, j)->parent = i;
        ImageComponent_get(components, j)->alpha = 0.0f;
        AmmoComponent_get(components, j)->size = 0;
    }
}


int get_slot(ComponentData* components, int i, int size) {
    PlayerComponent* player = PlayerComponent_get(components, i);

    sfVector2f rs = player->controller.right_stick;
    float angle = mod(polar_angle(rs) + 0.25 * M_PI, 2 * M_PI);

    return floor(size * angle / (2 * M_PI));
}


int get_attachment(ComponentData* components, int i) {
    PlayerComponent* player = PlayerComponent_get(components, i);
    int slot = get_slot(components, i, player->inventory_size);
    int item = player->inventory[slot];

    if (item != -1) {
        int size = components->item[item]->size;
        if (size > 0) {
            float slot_angle = 2 * M_PI / player->inventory_size;
            sfVector2f rs = player->controller.right_stick;
            float angle = polar_angle(rs) + 0.25 * M_PI;
            angle = mod(angle, slot_angle);

            return floor((size + 1) * angle / slot_angle) - 1;
        }
    }

    return -1;
}


void input(ComponentData* components, sfRenderWindow* window, int camera) {
    sfJoystick_update();

    for (int i = 0; i < components->entities; i++) {
        PlayerComponent* player = PlayerComponent_get(components, i);
        if (!player) continue;

        update_controller(components, window, camera, i);
        Controller controller = player->controller;

        switch (player->state) {
            case ON_FOOT:
                if (controller.buttons_down[BUTTON_LT]) {
                    player->state = MENU;
                }

                if (controller.buttons_pressed[BUTTON_RB]) {
                    player->state = PLAYER_PICK_UP;
                }

                if (controller.buttons_pressed[BUTTON_RT]) {
                    player->state = SHOOT;
                }

                if (controller.buttons_pressed[BUTTON_X]) {
                    WeaponComponent* weapon = WeaponComponent_get(components, player->inventory[player->item]);
                    if (weapon) {
                        player->state = RELOAD;
                    }
                }

                if (controller.buttons_pressed[BUTTON_A]) {
                    enter_vehicle(components, i);
                }

                if (controller.buttons_down[BUTTON_LB]) {
                    player->state = PLAYER_AMMO_MENU;
                }

                VehicleComponent* vehicle = VehicleComponent_get(components, player->vehicle);
                if (vehicle) {
                    if (vehicle->riders[0] == i) {
                        player->state = DRIVE;
                    } else {
                        player->state = PLAYER_PASSENGER;
                    }
                }

                break;
            case PLAYER_PICK_UP:
                break;
            case SHOOT:
                if (!controller.buttons_down[BUTTON_RT]) {
                    player->state = ON_FOOT;
                }

                break;
            case RELOAD:
                if (controller.buttons_down[BUTTON_LT]) {
                    player->state = MENU;
                }

                break;
            case DRIVE:
                if (controller.buttons_pressed[BUTTON_A]) {
                    exit_vehicle(components, i);
                    player->state = ON_FOOT;
                }

                break;
            case PLAYER_PASSENGER:
                if (controller.buttons_down[BUTTON_LT]) {
                    player->state = MENU;
                }

                if (controller.buttons_pressed[BUTTON_RT]) {
                    player->state = SHOOT;
                }

                if (controller.buttons_pressed[BUTTON_A]) {
                    exit_vehicle(components, i);
                    player->state = ON_FOOT;
                }

                break;
            case MENU:
                if (controller.buttons_pressed[BUTTON_RT]) {
                    player->state = MENU_GRAB;
                }

                if (!controller.buttons_down[BUTTON_LT]) {
                    player->state = ON_FOOT;
                }

                if (controller.buttons_pressed[BUTTON_RB]) {
                    drop_item(components, i);
                }

                break;
            case MENU_GRAB:
                if (controller.buttons_released[BUTTON_RT]) {
                    player->state = MENU_DROP;
                }

                break;
            case MENU_DROP:
                if (controller.buttons_down[BUTTON_LT]) {
                    player->state = MENU;
                } else {
                    player->state = ON_FOOT;
                }

                break;
            case PLAYER_AMMO_MENU:
                if (!controller.buttons_down[BUTTON_LB]) {
                    player->state = ON_FOOT;
                }

                break;
            case PLAYER_DEAD:
                break;
        }
    }
}


void update_players(ComponentData* components, ColliderGrid* grid, float time_step) {
    for (int i = 0; i < components->entities; i++) {
        PlayerComponent* player = PlayerComponent_get(components, i);
        if (!player) continue;

        PhysicsComponent* phys = components->physics[i];
        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        sfVector2f left_stick = player->controller.left_stick;
        sfVector2f right_stick = player->controller.right_stick;
        int slot = get_slot(components, i, player->inventory_size);
        int atch = get_attachment(components, i);

        int item = player->inventory[player->item];
        WeaponComponent* weapon = WeaponComponent_get(components, item);
        LightComponent* light = LightComponent_get(components, item);

        if (player->state != PLAYER_DEAD && HealthComponent_get(components, i)->health <= 0) {
            ImageComponent* image = ImageComponent_get(components, i);
            strcpy(image->filename, "player_dead");
            image->width = 2.0;
            image->texture_changed = true;
            change_layer(components, i, 2);
            player->state = PLAYER_DEAD;

            for (int j = 0; j < player->inventory_size; j++) {
                if (player->inventory[j] != -1) {
                    coord->angle = rand_angle();
                    player->item = j;
                    drop_item(components, i);
                }
            }

            for (int j = 1; j < player->ammo_size; j++) {
                AmmoComponent* ammo = AmmoComponent_get(components, player->ammo[j]);
                while (ammo->size > 0) {
                    sfVector2f pos = get_position(components, i);
                    int k = create_ammo(components, sum(pos, polar_to_cartesian(1.0f, rand_angle())), ammo->type);
                    int size = fminf(AmmoComponent_get(components, k)->size, ammo->size);
                    AmmoComponent* drop = AmmoComponent_get(components, k);
                    drop->size = size;
                    ammo->size -= size;
                }
            }
        }

        if (player->state != PLAYER_DEAD && player->state != DRIVE) {
            phys->acceleration = sum(phys->acceleration, mult(player->acceleration, left_stick));
            if (non_zero(right_stick)) {
                coord->angle = polar_angle(right_stick);
                if (coord->parent != -1) {
                    coord->angle -= get_angle(components, coord->parent);
                }
            }
        }
        
        switch (player->state) {
            case ON_FOOT:;
                sfVector2f pos = get_position(components, i);

                if (player->target != -1) {
                    ImageComponent_get(components, player->target)->outline = 0.0;
                }
                player->target = -1;

                float min_dist = 9999.0;

                List* list = get_entities(components, grid, pos, 1.0f);
                for (ListNode* current = list->head; current; current = current->next) {
                    int k = current->value;
                    if (k == -1) break;
                    if (!ItemComponent_get(components, k)) continue;
                    if (CoordinateComponent_get(components, k)->parent != -1) continue;

                    float d = dist(pos, get_position(components, k));
                    if (d < min_dist) {
                        player->target = k;
                        min_dist = d;
                    }
                }
                List_delete(list);

                if (player->target != -1) {
                    ImageComponent_get(components, player->target)->outline = 0.05f;
                }

                break;
            case PLAYER_PICK_UP:
                pick_up_item(components, grid, i);
                player->state = ON_FOOT;
                break;
            case SHOOT:
                if (weapon) {
                    shoot(components, grid, item);

                    if (weapon->reloading) {
                        player->state = RELOAD;
                    } else if (!weapon->automatic) {
                        player->state = ON_FOOT;
                    }
                } else if (light) {
                    light->enabled = !light->enabled;
                    player->state = ON_FOOT;
                }

                break;
            case RELOAD:
                if (weapon) {
                    reload(components, item);

                    if (!weapon->reloading) {
                        player->state = ON_FOOT;
                    }
                }

                break;
            case DRIVE:
                if (player->controller.joystick == -1) {
                    drive_vehicle(components, i, sign(left_stick.y), sign(left_stick.x), time_step);
                } else {
                    float gas = player->controller.right_trigger - player->controller.left_trigger;
                    drive_vehicle(components, i, gas, left_stick.x, time_step);
                }

                alert_enemies(components, grid, i, 10.0f);

                break;
            case MENU:
                phys->acceleration = sum(phys->acceleration, mult(player->acceleration, left_stick));
                
                if (light) {
                    light->enabled = false;
                }

                ItemComponent* itco = ItemComponent_get(components, item);
                if (weapon) {
                    weapon->reloading = false;
                } else if (itco) {
                    for (int j = 0; j < itco->size; j++) {
                        LightComponent* a = LightComponent_get(components, itco->attachments[j]);
                        if (a) {
                            a->enabled = false;
                        }
                    }
                }

                player->item = get_slot(components, i, player->inventory_size);

                int new_item = player->inventory[player->item];
                if (new_item != item) {
                    if (item != -1) {
                        ImageComponent_get(components, item)->alpha = 0.0;
                    }
                    if (new_item != -1) {
                        ImageComponent_get(components, new_item)->alpha = 1.0;
                    }
                }

                break;
            case MENU_GRAB:
                if (player->grabbed_item == -1 && player->inventory[slot] != -1) {
                    if (atch == -1) {
                        player->grabbed_item = item;
                    } else if (components->item[item]->attachments[atch] != -1) {
                        player->grabbed_item = components->item[item]->attachments[atch];
                    }
                }

                break;
            case MENU_DROP:
                if (player->grabbed_item != -1) {
                    if (player->inventory[slot] == -1) {
                        replace(player->grabbed_item, -1, player->inventory, player->inventory_size);
                        for (int j = 0; j < player->inventory_size; j++) {
                            if (player->inventory[j] == -1) continue;
                            ItemComponent* it = components->item[player->inventory[j]];
                            replace(player->grabbed_item, -1, it->attachments, it->size);
                        }
                        player->inventory[slot] = player->grabbed_item;
                        components->coordinate[player->grabbed_item]->parent = i;
                    } else if (atch != -1) {
                        if (player->inventory[slot] != player->grabbed_item && components->item[player->inventory[slot]]->attachments[atch] == -1) {
                            replace(player->grabbed_item, -1, player->inventory, player->inventory_size);
                            for (int j = 0; j < player->inventory_size; j++) {
                                if (player->inventory[j] == -1) continue;
                                ItemComponent* it = components->item[player->inventory[j]];
                                replace(player->grabbed_item, -1, it->attachments, it->size);
                            }
                            components->item[player->inventory[slot]]->attachments[atch] = player->grabbed_item;
                            components->coordinate[player->grabbed_item]->parent = player->inventory[slot];
                        }
                    }
                    player->grabbed_item = -1;
                }

                break;
            case PLAYER_AMMO_MENU:
                break;
            case PLAYER_DEAD:;
                ColliderComponent* col = ColliderComponent_get(components, i);
                if (col) {
                    col->group = ITEMS;
                    if (phys->speed == 0.0) {
                        clear_grid(components, grid, i);
                        ColliderComponent_remove(components, i);
                    }
                }

                break;
        }
    }
}
