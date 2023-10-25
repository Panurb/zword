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
#include "health.h"


int create_player(ComponentData* components, sfVector2f pos, float angle) {
    int joystick = -1;
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, angle);
    ImageComponent_add(components, i, "player", 1.0, 1.0, LAYER_PLAYERS);
    PhysicsComponent* phys = PhysicsComponent_add(components, i, 1.0f);
    phys->bounce = 0.0f;
    phys->max_speed = 5.0;
    ColliderComponent_add_circle(components, i, 0.5, GROUP_PLAYERS);
    PlayerComponent* player = PlayerComponent_add(components, i, joystick);
    ParticleComponent_add_type(components, i, PARTICLE_BLOOD, 0.0f);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 100, "player_dead", "blood", "");
    SoundComponent_add(components, i, "squish");

    int j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) { 0.75f, 0.0f }, 0.0f)->parent = i;
    ImageComponent_add(components, j, "", 0.0f, 0.0f, LAYER_WEAPONS);
    AnimationComponent* animation = AnimationComponent_add(components, j, 1);
    animation->play_once = true;
    animation->framerate = 0.0f;
    player->arms = j;

    // TODO: why?
    // JointComponent_add(components, i, -1, 0.0f, 0.0f, INFINITY);

    // for (AmmoType type = AMMO_PISTOL; type <= AMMO_SHOTGUN; type++) {
    //     int j = create_ammo(components, zeros(), type);
    //     player->ammo[type] = j;
    //     CoordinateComponent_get(components, j)->parent = i;
    //     ColliderComponent_remove(components, j);
    //     ImageComponent_get(components, j)->alpha = 0.0f;
    //     AmmoComponent_get(components, j)->size = 0;
    // }

    return i;
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
        ItemComponent* itco = ItemComponent_get(components, item);
        WeaponComponent* weapon = WeaponComponent_get(components, item);
        LightComponent* light = LightComponent_get(components, item);

        if (player->state != PLAYER_DEAD && player->state != PLAYER_DRIVE) {
            phys->acceleration = sum(phys->acceleration, mult(player->acceleration, left_stick));
            if (non_zero(right_stick)) {
                coord->angle = polar_angle(right_stick);
                if (coord->parent != -1) {
                    coord->angle -= get_angle(components, coord->parent);
                }
            }
        }

        if (weapon) {
            switch (weapon->ammo_type) {
                case AMMO_PISTOL:
                    change_texture(components, player->arms, "arms_pistol", 0.0f, 0.0f);
                    break;
                case AMMO_RIFLE:
                    change_texture(components, player->arms, "arms_assault_rifle", 0.0f, 0.0f);
                    break;
                case AMMO_SHOTGUN:
                    change_texture(components, player->arms, "arms_shotgun", 0.0f, 0.0f);
                    break;
                default:
                    change_texture(components, player->arms, "arms_axe", 2.0f, 2.0f);
                    break;
            }
        } else {
            if (itco) {
                change_texture(components, player->arms, "arms", 0.0f, 0.0f);
            } else {
                change_texture(components, player->arms, "", 0.0f, 0.0f);
            }
        }
        
        switch (player->state) {
            case PLAYER_ON_FOOT:;
                sfVector2f pos = get_position(components, i);

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

                if (!weapon && light) {
                    light->enabled = true;
                }

                if (itco) {
                    for (int j = 0; j < itco->size; j++) {
                        LightComponent* a = LightComponent_get(components, itco->attachments[j]);
                        if (a) {
                            a->enabled = true;
                        }
                    }
                }

                player->use_timer = 0.0f;
                break;
            case PLAYER_PICK_UP:
                pick_up_item(components, grid, i);
                player->state = PLAYER_ON_FOOT;
                break;
            case PLAYER_SHOOT:
                if (weapon) {
                    shoot(components, grid, item);

                    if (weapon->reloading) {
                        player->state = PLAYER_RELOAD;
                    } else if (!weapon->automatic) {
                        player->state = PLAYER_ON_FOOT;
                    }
                } else if (itco) {
                    use_item(components, grid, item, time_step);
                }

                break;
            case PLAYER_RELOAD:
                if (weapon) {
                    reload(components, item);

                    if (!weapon->reloading) {
                        player->state = PLAYER_ON_FOOT;
                    }
                }

                break;
            case PLAYER_ENTER:
                if (!enter_vehicle(components, grid, i)) {
                    player->state = PLAYER_ON_FOOT;
                }
                break;
            case PLAYER_DRIVE:
                if (player->controller.joystick == -1) {
                    drive_vehicle(components, i, sign(left_stick.y), sign(left_stick.x));
                } else {
                    float gas = player->controller.right_trigger - player->controller.left_trigger;
                    drive_vehicle(components, i, gas, left_stick.x);
                }

                alert_enemies(components, grid, i, 10.0f);

                break;
            case PLAYER_PASSENGER:
                if (!weapon && light) {
                    light->enabled = true;
                }

                if (itco) {
                    for (int j = 0; j < itco->size; j++) {
                        LightComponent* a = LightComponent_get(components, itco->attachments[j]);
                        if (a) {
                            a->enabled = true;
                        }
                    }
                }

                break;
            case PLAYER_MENU:
                phys->acceleration = sum(phys->acceleration, mult(player->acceleration, left_stick));
                
                if (light) {
                    light->enabled = false;
                }
                
                if (weapon) {
                    weapon->reloading = false;
                }

                if (itco) {
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
                        ImageComponent_get(components, item)->alpha = 0.0f;
                    }
                    if (new_item != -1) {
                        itco = ItemComponent_get(components, new_item);

                        if (!WeaponComponent_get(components, new_item)) {
                            ImageComponent_get(components, new_item)->alpha = 1.0f;
                        }
                        for (int j = 0; j < itco->size; j++) {
                            int k = itco->attachments[j];
                            if (k != -1) {
                                ImageComponent_get(components, k)->alpha = 0.0f;
                            }
                        }
                    }
                }

                break;
            case PLAYER_MENU_GRAB:
                if (player->grabbed_item == -1 && player->inventory[slot] != -1) {
                    if (atch == -1) {
                        player->grabbed_item = item;
                    } else if (components->item[item]->attachments[atch] != -1) {
                        player->grabbed_item = components->item[item]->attachments[atch];
                    }
                }

                break;
            case PLAYER_MENU_DROP:
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
                    col->group = GROUP_ITEMS;
                    if (phys->speed == 0.0) {
                        clear_grid(components, grid, i);
                        // ColliderComponent_remove(components, i);
                    }
                }

                break;
        }
    }
}
