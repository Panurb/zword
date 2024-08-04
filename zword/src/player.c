#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

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


int create_player(Vector2f pos, float angle) {
    int i = create_entity();

    CoordinateComponent_add(i, pos, angle);
    ImageComponent_add(i, "player", 1.0, 1.0, LAYER_PLAYERS);
    PhysicsComponent* phys = PhysicsComponent_add(i, 1.0f);
    phys->bounce = 0.0f;
    phys->max_speed = 5.0;
    ColliderComponent_add_circle(i, 0.5, GROUP_PLAYERS);
    PlayerComponent* player = PlayerComponent_add(i);
    ParticleComponent_add_type(i, PARTICLE_BLOOD, 0.0f);
    WaypointComponent_add(i);
    HealthComponent_add(i, 100, "player_dead", "blood", "");
    SoundComponent_add(i, "squish");

    int j = create_entity();
    CoordinateComponent_add(j, (Vector2f) { 0.75f, 0.0f }, 0.0f)->parent = i;
    ImageComponent_add(j, "", 0.0f, 0.0f, LAYER_WEAPONS);
    AnimationComponent* animation = AnimationComponent_add(j, 1);
    animation->play_once = true;
    animation->framerate = 0.0f;
    player->arms = j;

    return i;
}


int get_slot(int i, int size) {
    PlayerComponent* player = PlayerComponent_get(i);

    Vector2f rs = player->controller.right_stick;
    float angle = mod(polar_angle(rs) + 0.25 * M_PI, 2 * M_PI);

    return floor(size * angle / (2 * M_PI));
}


int get_attachment(int i) {
    PlayerComponent* player = PlayerComponent_get(i);
    int slot = get_slot(i, player->inventory_size);
    int item = player->inventory[slot];

    if (item != -1) {
        int size = ItemComponent_get(item)->size;
        if (size > 0) {
            float slot_angle = 2 * M_PI / player->inventory_size;
            Vector2f rs = player->controller.right_stick;
            float angle = polar_angle(rs) + 0.25 * M_PI;
            angle = mod(angle, slot_angle);

            return floor((size + 1) * angle / slot_angle) - 1;
        }
    }

    return -1;
}


void update_players(float time_step) {
    for (int i = 0; i < game_data->components->entities; i++) {
        PlayerComponent* player = PlayerComponent_get(i);
        if (!player) continue;

        PhysicsComponent* phys = PhysicsComponent_get(i);
        CoordinateComponent* coord = CoordinateComponent_get(i);
        Vector2f left_stick = player->controller.left_stick;
        Vector2f right_stick = player->controller.right_stick;
        int slot = get_slot(i, player->inventory_size);
        int atch = get_attachment(i);

        int item = player->inventory[player->item];
        ItemComponent* itco = ItemComponent_get(item);
        WeaponComponent* weapon = WeaponComponent_get(item);
        LightComponent* light = LightComponent_get(item);
        ImageComponent* image = ImageComponent_get(item);

        if (player->state != PLAYER_DEAD && player->state != PLAYER_DRIVE) {
            phys->acceleration = sum(phys->acceleration, mult(player->acceleration, left_stick));
            if (non_zero(right_stick)) {
                coord->angle = polar_angle(right_stick);
                if (coord->parent != -1) {
                    coord->angle -= get_angle(coord->parent);
                }
            }

            player->money_timer = fmaxf(player->money_timer - time_step, 0.0f);
        }

        if (weapon) {
            char buffer[256];
            snprintf(buffer, 256, "arms_%s", image->filename);

            if (strcmp(image->filename, "axe") == 0) {
                change_texture(player->arms, buffer, 2.0f, 2.0f);
            } else if (strcmp(image->filename, "sword") == 0) {
                change_texture(player->arms, buffer, 2.0f, 3.0f);
            } else {
                change_texture(player->arms, buffer, 0.0f, 0.0f);
            }
        } else {
            if (itco) {
                change_texture(player->arms, "arms", 0.0f, 0.0f);
            } else {
                change_texture(player->arms, "", 0.0f, 0.0f);
            }
        }
        
        switch (player->state) {
            case PLAYER_ON_FOOT:;
                Vector2f pos = get_position(i);

                player->target = -1;

                float min_dist = 9999.0;

                List* list = get_entities(pos, 1.0f);
                for (ListNode* current = list->head; current; current = current->next) {
                    int k = current->value;
                    if (k == -1) break;
                    if (!ItemComponent_get(k)) continue;
                    if (CoordinateComponent_get(k)->parent != -1) continue;

                    float d = dist(pos, get_position(k));
                    if (d < min_dist) {
                        player->target = k;
                        min_dist = d;
                    }
                }
                List_delete(list);

                if (itco) {
                    for (int j = 0; j < itco->size; j++) {
                        LightComponent* a = LightComponent_get(itco->attachments[j]);
                        if (a) {
                            a->enabled = true;
                        }
                    }
                }

                player->use_timer = 0.0f;
                break;
            case PLAYER_PICK_UP:
                pick_up_item(i);
                player->state = PLAYER_ON_FOOT;
                break;
            case PLAYER_SHOOT:
                if (weapon) {
                    attack(item);

                    if (weapon->reloading) {
                        player->state = PLAYER_RELOAD;
                    } else if (!weapon->automatic) {
                        player->state = PLAYER_ON_FOOT;
                    }
                } else if (itco) {
                    use_item(item, time_step);
                    player->state = PLAYER_ON_FOOT;
                }

                break;
            case PLAYER_RELOAD:
                if (weapon) {
                    player->target = -1;
                    reload(item);

                    if (!weapon->reloading) {
                        player->state = PLAYER_ON_FOOT;
                    }
                }

                break;
            case PLAYER_ENTER:
                if (!enter_vehicle(i)) {
                    player->state = PLAYER_ON_FOOT;
                }
                break;
            case PLAYER_DRIVE:
                if (player->controller.joystick == -1) {
                    drive_vehicle(i, sign(left_stick.y), sign(left_stick.x));
                } else {
                    float gas = player->controller.right_trigger - player->controller.left_trigger;
                    drive_vehicle(i, gas, left_stick.x);
                }

                alert_enemies(i, 10.0f);

                break;
            case PLAYER_PASSENGER:
                if (!weapon && light) {
                    light->enabled = true;
                }

                if (itco) {
                    for (int j = 0; j < itco->size; j++) {
                        LightComponent* a = LightComponent_get(itco->attachments[j]);
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
                        LightComponent* a = LightComponent_get(itco->attachments[j]);
                        if (a) {
                            a->enabled = false;
                        }
                    }
                }

                player->item = get_slot(i, player->inventory_size);

                int new_item = player->inventory[player->item];

                if (new_item != item) {
                    if (item != -1) {
                        ImageComponent_get(item)->alpha = 0.0f;
                    }
                    if (new_item != -1) {
                        itco = ItemComponent_get(new_item);

                        if (!WeaponComponent_get(new_item)) {
                            ImageComponent_get(new_item)->alpha = 1.0f;
                        }
                        for (int j = 0; j < itco->size; j++) {
                            int k = itco->attachments[j];
                            if (k != -1) {
                                ImageComponent_get(k)->alpha = 0.0f;
                            }
                        }
                    }
                }

                break;
            case PLAYER_MENU_GRAB:
                if (player->grabbed_item == -1 && player->inventory[slot] != -1) {
                    if (atch == -1) {
                        player->grabbed_item = item;
                    } else if (itco->attachments[atch] != -1) {
                        player->grabbed_item = itco->attachments[atch];
                    }
                }

                break;
            case PLAYER_MENU_DROP:
                if (player->grabbed_item != -1) {
                    if (player->inventory[slot] == -1) {
                        replace(player->grabbed_item, -1, player->inventory, player->inventory_size);
                        for (int j = 0; j < player->inventory_size; j++) {
                            if (player->inventory[j] == -1) continue;
                            ItemComponent* it = ItemComponent_get(player->inventory[j]);
                            replace(player->grabbed_item, -1, it->attachments, it->size);
                        }
                        player->inventory[slot] = player->grabbed_item;
                        CoordinateComponent_get(player->grabbed_item)->parent = i;
                    } else if (atch != -1) {
                        ItemComponent* item_slot = ItemComponent_get(player->inventory[slot]);
                        if (player->inventory[slot] != player->grabbed_item && item_slot->attachments[atch] == -1) {
                            if (is_attachment(player->grabbed_item)) {
                                replace(player->grabbed_item, -1, player->inventory, player->inventory_size);
                                for (int j = 0; j < player->inventory_size; j++) {
                                    if (player->inventory[j] == -1) continue;
                                    ItemComponent* it = ItemComponent_get(player->inventory[j]);
                                    replace(player->grabbed_item, -1, it->attachments, it->size);
                                }
                                item_slot->attachments[atch] = player->grabbed_item;
                                CoordinateComponent_get(player->grabbed_item)->parent = player->inventory[slot];
                            }
                        }
                    }
                    player->grabbed_item = -1;
                }

                break;
            case PLAYER_AMMO_MENU:
                break;
            case PLAYER_DEAD:;
                ColliderComponent* col = ColliderComponent_get(i);
                if (col) {
                    col->group = GROUP_ITEMS;
                    if (phys->speed == 0.0) {
                        clear_grid(i);
                        // ColliderComponent_remove(i);
                    }
                }

                break;
        }
    }
}


void add_money(int entity, int amount) {
    if (amount == 0) return;
    
    PlayerComponent* player = PlayerComponent_get(entity);
    player->money += amount;
    player->money_increment = amount;
    player->money_timer = 1.0f;
}
