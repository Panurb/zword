#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "player.h"
#include "weapon.h"
#include "game.h"
#include "input.h"
#include "app.h"


void draw_menu_slot(int camera, int entity, int slot, float offset, float alpha) {
    PlayerComponent* player = PlayerComponent_get(entity);
    Vector2f pos = get_position_interpolated(entity, app.delta);

    float gap = 0.2;
    float slice = (2 * M_PI / player->inventory_size);

    Color color = COLOR_WHITE;
    color.a = alpha * 255;

    if (alpha == 0.0) {
        draw_slice_outline(camera, pos, 1.0 + offset, 2.0 + offset, slot * slice, slice - gap);
    } else {
        draw_slice(camera, pos, 1.0 + offset, 2.0 + offset, slot * slice, slice - gap, color);
    }

    int i = player->inventory[slot];
    if (i != -1) {
        ItemComponent* item = ItemComponent_get(i);

        ImageComponent* image = ImageComponent_get(i);

        Vector2f r = polar_to_cartesian(1.5 + offset, slot * slice - 0.5 * slice + 0.5 * slice / (item->size + 1));
        float angle = (slot - 1) * 0.5f * M_PI;

        if (i == player->grabbed_item) {
            r = mult(2.5f, player->controller.right_stick);
            angle = polar_angle(r) - 0.5f * M_PI;
        }

        draw_sprite(camera, image->texture_index, image->width, image->height, 0, sum(pos, r), angle, ones(), 1.0f);
    }
}


void draw_menu_attachment(int camera, int entity, int slot, int atch, float offset, float alpha) {
    PlayerComponent* player = PlayerComponent_get(entity);
    Vector2f pos = get_position_interpolated(entity, app.delta);
    ItemComponent* item = ItemComponent_get(player->inventory[slot]);

    float gap = 0.2;
    float slice = (2 * M_PI / player->inventory_size);

    float spread = (slice - 2 * gap - 0.5 * item->size * gap) / (item->size + 1);
    float angle = (slot - 0.5) * slice + (atch + 1.5) * spread + (0.5 * atch + 1.5) * gap;
    draw_slice_outline(camera, pos, 1.2 + offset, 1.8 + offset, angle, spread);

    int a = item->attachments[atch];
    if (a != -1) {
        Color color = COLOR_WHITE;
        color.a = alpha * 255;

        draw_slice(camera, pos, 1.2 + offset, 1.8 + offset, angle, spread, color);

        ImageComponent* image = ImageComponent_get(a);

        Vector2f r = polar_to_cartesian(1.5 + offset, slot * slice - 0.5 * slice + (atch + 1.5) * slice / (item->size + 1));

        if (a == player->grabbed_item) {
            r = mult(2.5, player->controller.right_stick);
        }

        float angle = (slot - 1) * 0.5f * M_PI;
        Vector2f scale = mult(0.75, ones());
        draw_sprite(camera, image->texture_index, image->width, image->height, 0, sum(pos, r), angle, scale, 1.0f);
    }
}


void draw_ammo_slot(int camera, int entity, int slot, float offset, float alpha) {
    PlayerComponent* player = PlayerComponent_get(entity);
    Vector2f pos = get_position_interpolated(entity, app.delta);

    float gap = 0.2f;
    float slice = (2 * M_PI / (player->ammo_size - 1));

    Color color = COLOR_WHITE;
    color.a = alpha * 255;

    if (alpha == 0.0f) {
        draw_slice_outline(camera, pos, 1.0f + offset, 2.0f + offset, slot * slice, slice - gap);
    } else {
        draw_slice(camera, pos, 1.0f + offset, 2.0f + offset, slot * slice, slice - gap, color);
    }

    int i = player->ammo[slot + 1];
    if (i != -1) {
        ImageComponent* image = ImageComponent_get(i);
        draw_sprite(camera, image->texture_index, image->width, image->height, 0, sum(pos, polar_to_cartesian(1.5f + offset, slot * slice - 0.1f * M_PI)), 0.0f, ones(), 1.0f);

        char buffer[20];
        snprintf(buffer, 20, "%i", AmmoComponent_get(i)->size);
        draw_text(camera, sum(pos, polar_to_cartesian(1.5f + offset, slot * slice + 0.1f * M_PI)), buffer, 20, COLOR_WHITE);
    }
}


void draw_ammo_menu(int camera, int entity) {
    PlayerComponent* player = PlayerComponent_get(entity);

    int slot = get_slot(entity, player->ammo_size - 1);

    for (int i = 0; i < player->ammo_size - 1; i++) {
        float offset = (i == slot) ? 0.2f : 0.0f;
        float alpha = (player->ammo[i] == 0) ? 0.25f : 0.5f;
        draw_ammo_slot(camera, entity, i, offset, alpha);
    }
}


void draw_item_use(int camera, int entity) {
    Vector2f position = get_position_interpolated(entity, app.delta);
    PlayerComponent* player = PlayerComponent_get(entity);
    int i = player->inventory[player->item];
    ItemComponent* item = ItemComponent_get(i);
    if (!item) return;

    if (item->use_time == 0.0f) return;
    
    float x = 2.0f * M_PI * player->use_timer / item->use_time;
    draw_slice(camera, position, 1.0f, 1.2f, -0.5f * x + M_PI_2, x, COLOR_WHITE);
}


void draw_money(int camera, int entity) {
    PlayerComponent* player = PlayerComponent_get(entity);
    Vector2f position = sum(get_position_interpolated(entity, app.delta), 
        vec(0.0f, (1.0f - player->money_timer) * sign(player->money_increment)));

    char buffer[256];
    snprintf(buffer, 256, "%d", player->money_increment);
    if (player->money_timer > 0.0f) {
        Color color = get_color(1.0f, 1.0f, 0.0f, player->money_timer);
        draw_text(camera, position, buffer, 20, color);
    }
}


void draw_keys(Entity camera, Entity player) {
    PlayerComponent* pco = PlayerComponent_get(player);
    Vector2f pos = get_position_interpolated(player, app.delta);

    for (int i = 0; i < pco->keys_size; i++) {
        int key = pco->keys[i];
        if (key == NULL_ENTITY) continue;

        ImageComponent* image = ImageComponent_get(key);
        Vector2f r = vec(0.4f * (i - 0.5 * pco->keys_size), 0.0f);
        draw_sprite(camera, image->texture_index, image->width, image->height, 0, sum(pos, r), 0.0f, ones(), 1.0f);
    }
} 


void draw_hud(int camera) {
    for (int i = 0; i < game_data->components->entities; i++) {
        if (!CoordinateComponent_get(i)) continue;

        Vector2f position = get_position_interpolated(i, app.delta);

        TextComponent* text = TextComponent_get(i);
        if (text) {
            float r = norm(diff(get_position(i), get_position(camera)));
            if (r < 8.0f) {
                float alpha = 1.0f - 0.125f * r;
                Color color = text->color;
                color.a = alpha * 255;
                draw_text(camera, position, text->string, text->size, color);
            }
        }

        PlayerComponent* player = PlayerComponent_get(i);
        if (!player) continue;

        int slot = get_slot(i, player->inventory_size);
        int atch = get_attachment(i);

        int item = player->inventory[player->item];
        WeaponComponent* weapon = NULL;
        if (item != -1) {
            weapon = WeaponComponent_get(item);
        }

        Vector2f pos;
        if (player->controller.joystick == -1) {
            pos = get_mouse_position(camera);
        } else {
            pos = polar_to_cartesian(fmaxf(2.0f, 5.0f * norm(player->controller.right_stick)), get_angle(i));
            pos = sum(position, pos);
        }

        if (player->state != PLAYER_DEAD) {
            if (weapon) {
                float r = fmaxf(dist(pos, position) * tanf(0.5f * fmaxf(weapon->spread, weapon->recoil)), 0.1f);
                if (weapon->ammo_type == AMMO_MELEE) {
                    r = 0.1f;
                }

                if (r > 0.1f) {
                    draw_circle_outline(camera, pos, r, 0.03f, COLOR_WHITE);
                } else {
                    draw_circle(camera, pos, r, COLOR_WHITE);
                }
            } else {
                draw_circle(camera, pos, 0.1f, COLOR_WHITE);
            }

            HealthComponent* health = HealthComponent_get(i);
            float x = 2.0f * M_PI * (health->max_health - health->health) / health->max_health;
            if (health->health != health->max_health) {
                draw_slice(camera, position, 0.5f, 0.6f, M_PI_2 - 0.5f * x, x, COLOR_RED);
            }

            if (game_data->game_mode == MODE_SURVIVAL) {
                draw_money(camera, i);
            }
        }

        char buffer[128];
        switch (player->state) {
            case PLAYER_ON_FOOT:
                break;
            case PLAYER_INTERACT:
                break;
            case PLAYER_SHOOT:
                draw_item_use(camera, i);
                break;
            case PLAYER_RELOAD:;
                int akimbo = get_akimbo(item);
                float prog = 2 * M_PI * (1.0 - weapon->cooldown / ((1 + akimbo) * weapon->reload_time));
                draw_slice(camera, position, 0.75, 1.0, 0.5 * M_PI - 0.5 * prog, prog, COLOR_WHITE);

                break;
            case PLAYER_ENTER:
                break;
            case PLAYER_DRIVE:
                // VehicleComponent* vehicle = VehicleComponent_get(player->vehicle);
                // if (vehicle) {
                //     draw_slice(camera, NULL, 50, get_position(player->vehicle), 1.0, 1.2, 0.5 * M_PI, vehicle->fuel / vehicle->max_fuel * M_PI, COLOR_WHITE);
                // }

                break;
            case PLAYER_PASSENGER:
                break;
            case PLAYER_MENU:
            case PLAYER_MENU_DROP:
            case PLAYER_MENU_GRAB:
                snprintf(buffer, 128, "%d", player->money);
                draw_text(camera, position, buffer, 20, COLOR_YELLOW);

                for (int j = 0; j < player->inventory_size; j++) {
                    float offset = 0.0;
                    float alpha = 0.5;

                    if (j == slot) {
                        offset = 0.2;
                    }
                    
                    if (player->inventory[j] == -1) {
                        alpha = 0.25;
                    }

                    if (player->grabbed_item != -1 && player->inventory[j] == player->grabbed_item) {
                        alpha = 0.0;
                    }
                        
                    draw_menu_slot(camera, i, j, offset, alpha);

                    if (player->inventory[j] != -1) {
                        ItemComponent* item = game_data->components->item[player->inventory[j]];
                        for (int k = 0; k < item->size; k++) {
                            if (j == slot) {
                                offset = 0.2;
                                if (k == atch && item->attachments[k] == -1) {
                                    offset = 0.4;
                                }
                            }

                            draw_menu_attachment(camera, i, j, k, offset, alpha);
                        }
                    }
                }
                draw_keys(camera, i);

                break;
            case PLAYER_AMMO_MENU:
                snprintf(buffer, 128, "%d", player->money);
                draw_text(camera, position, buffer, 20, COLOR_YELLOW);
                
                draw_ammo_menu(camera, i);
                break;
            case PLAYER_DEAD:
                break;
        }
    }
}
