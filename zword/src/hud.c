#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "player.h"
#include "weapon.h"


void draw_menu_slot(ComponentData* components, sfRenderWindow* window, int camera, int entity, int slot, float offset, float alpha) {
    PlayerComponent* player = PlayerComponent_get(entity);
    sfVector2f pos = get_position(components, entity);

    float gap = 0.2;
    float slice = (2 * M_PI / player->inventory_size);

    sfColor color = sfConvexShape_getFillColor(player->shape);
    color.a = alpha * 255;
    sfConvexShape_setFillColor(player->shape, color);

    if (alpha == 0.0) {
        draw_slice_outline(window, components, camera, player->line, pos, 1.0 + offset, 2.0 + offset, slot * slice, slice - gap);
    } else {
        draw_slice(window, components, camera, NULL, 50, pos, 1.0 + offset, 2.0 + offset, slot * slice, slice - gap, color);
    }

    int i = player->inventory[slot];
    if (i != -1) {
        ItemComponent* item = ItemComponent_get(i);

        sfSprite* sprite = ImageComponent_get(i)->sprite;

        sfVector2f r = polar_to_cartesian(1.5 + offset, slot * slice - 0.5 * slice + 0.5 * slice / (item->size + 1));
        float angle = (slot - 1) * 0.5f * M_PI;

        if (i == player->grabbed_item) {
            r = mult(2.5f, player->controller.right_stick);
            angle = polar_angle(r) - 0.5f * M_PI;
        }

        float alpha = sfSprite_getColor(sprite).a;
        sfSprite_setColor(sprite, sfWhite);
        draw_sprite(window, components, camera, sprite, sum(pos, r), angle, ones(), 0);
        sfSprite_setColor(sprite, get_color(1.0f, 1.0f, 1.0f, alpha));
    }
}


void draw_menu_attachment(ComponentData* components, sfRenderWindow* window, int camera, int entity, int slot, int atch, float offset, float alpha) {
    PlayerComponent* player = PlayerComponent_get(entity);
    sfVector2f pos = get_position(components, entity);
    ItemComponent* item = components->item[player->inventory[slot]];

    float gap = 0.2;
    float slice = (2 * M_PI / player->inventory_size);

    float spread = (slice - 2 * gap - 0.5 * item->size * gap) / (item->size + 1);
    float angle = (slot - 0.5) * slice + (atch + 1.5) * spread + (0.5 * atch + 1.5) * gap;
    draw_slice_outline(window, components, camera, player->line, pos, 1.2 + offset, 1.8 + offset, angle, spread);

    int a = item->attachments[atch];
    if (a != -1) {
        sfColor color = sfConvexShape_getFillColor(player->shape);
        color.a = alpha * 255;
        sfConvexShape_setFillColor(player->shape, color);

        draw_slice(window, components, camera, NULL, 50, pos, 1.2 + offset, 1.8 + offset, angle, spread, color);

        sfSprite* sprite = ImageComponent_get(a)->sprite;

        sfVector2f r = polar_to_cartesian(1.5 + offset, slot * slice - 0.5 * slice + (atch + 1.5) * slice / (item->size + 1));

        if (a == player->grabbed_item) {
            r = mult(2.5, player->controller.right_stick);
        }

        draw_sprite(window, components, camera, sprite, sum(pos, r), (slot - 1) * 0.5f * M_PI, mult(0.75, ones()), 0);
    }
}


void draw_ammo_slot(ComponentData* components, sfRenderWindow* window, int camera, int entity, int slot, float offset, float alpha) {
    PlayerComponent* player = PlayerComponent_get(entity);
    sfVector2f pos = get_position(components, entity);

    float gap = 0.2f;
    float slice = (2 * M_PI / (player->ammo_size - 1));

    sfColor color = sfConvexShape_getFillColor(player->shape);
    color.a = alpha * 255;
    sfConvexShape_setFillColor(player->shape, color);

    if (alpha == 0.0f) {
        draw_slice_outline(window, components, camera, player->line, pos, 1.0f + offset, 2.0f + offset, slot * slice, slice - gap);
    } else {
        draw_slice(window, components, camera, NULL, 50, pos, 1.0f + offset, 2.0f + offset, slot * slice, slice - gap, color);
    }

    int i = player->ammo[slot + 1];
    if (i != -1) {
        sfSprite* sprite = ImageComponent_get(i)->sprite;
        float alpha = sfSprite_getColor(sprite).a;
        sfSprite_setColor(sprite, sfWhite);
        draw_sprite(window, components, camera, sprite, sum(pos, polar_to_cartesian(1.5f + offset, slot * slice - 0.1f * M_PI)), 0.0f, ones(), 0);
        sfSprite_setColor(sprite, get_color(1.0f, 1.0f, 1.0f, alpha));

        char buffer[20];
        snprintf(buffer, 20, "%i", AmmoComponent_get(i)->size);
        draw_text(window, components, camera, NULL, sum(pos, polar_to_cartesian(1.5f + offset, slot * slice + 0.1f * M_PI)), buffer, 20, sfWhite);
    }
}


void draw_ammo_menu(ComponentData* components, sfRenderWindow* window, int camera, int entity) {
    PlayerComponent* player = PlayerComponent_get(entity);

    int slot = get_slot(components, entity, player->ammo_size - 1);

    for (int i = 0; i < player->ammo_size - 1; i++) {
        float offset = (i == slot) ? 0.2f : 0.0f;
        float alpha = (player->ammo[i] == 0) ? 0.25f : 0.5f;
        draw_ammo_slot(components, window, camera, entity, i, offset, alpha);
    }
}


void draw_item_use(ComponentData* components, sfRenderWindow* window, int camera, int entity) {
    sfVector2f position = get_position(components, entity);
    PlayerComponent* player = PlayerComponent_get(entity);
    int i = player->inventory[player->item];
    ItemComponent* item = ItemComponent_get(i);
    if (!item) return;

    float x = 2.0f * M_PI * player->use_timer / item->use_time;
    draw_slice(window, components, camera, NULL, 50, position, 1.0f, 1.2f, -0.5f * x + M_PI_2, x, sfWhite);
}


void draw_money(ComponentData* components, sfRenderWindow* window, int camera, int entity) {
    PlayerComponent* player = PlayerComponent_get(entity);
    sfVector2f position = sum(get_position(components, entity), 
        vec(0.0f, (1.0f - player->money_timer) * sign(player->money_increment)));

    char buffer[256];
    snprintf(buffer, 256, "%d", player->money_increment);
    if (player->money_timer > 0.0f) {
        sfColor color = get_color(1.0f, 1.0f, 0.0f, player->money_timer);
        draw_text(window, components, camera, NULL, position, buffer, 20, color);
    }
}


void draw_hud(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        if (!CoordinateComponent_get(i)) continue;

        sfVector2f position = get_position(components, i);

        TextComponent* text = TextComponent_get(i);
        if (text) {
            float r = norm(diff(position, get_position(components, camera)));
            if (r < 8.0f) {
                float alpha = 1.0f - 0.125f * r;
                sfColor color = text->color;
                color.a = alpha * 255;
                draw_text(window, components, camera, NULL, position, text->string, text->size, color);
            }
        }

        PlayerComponent* player = PlayerComponent_get(i);
        if (!player) continue;

        int slot = get_slot(components, i, player->inventory_size);
        int atch = get_attachment(components, i);

        int item = player->inventory[player->item];
        WeaponComponent* weapon = NULL;
        if (item != -1) {
            weapon = components->weapon[item];
        }

        sfVector2f pos;
        if (player->controller.joystick == -1) {
            pos = screen_to_world(components, camera, sfMouse_getPosition((sfWindow*) window));
        } else {
            pos = polar_to_cartesian(fmaxf(2.0f, 5.0f * norm(player->controller.right_stick)), get_angle(components, i));
            pos = sum(position, pos);
        }

        if (player->state != PLAYER_DEAD) {
            if (weapon) {
                float r = fmaxf(dist(pos, position) * tanf(0.5f * fmaxf(weapon->spread, weapon->recoil)), 0.1f);
                if (weapon->ammo_type == AMMO_MELEE) {
                    r = 0.1f;
                }
                sfColor color = (r == 0.1f) ? sfWhite : sfTransparent;
                draw_circle(window, components, camera, player->crosshair, pos, r, color);
            } else {
                draw_circle(window, components, camera, player->crosshair, pos, 0.1f, sfWhite);
            }

            HealthComponent* health = HealthComponent_get(i);
            float x = 2.0f * M_PI * (health->max_health - health->health) / health->max_health;
            if (health->health != health->max_health) {
                draw_slice(window, components, camera, NULL, 50, position, 0.5f, 0.6f, M_PI_2 - 0.5f * x, x, sfRed);
            }

            draw_money(components, window, camera, i);
        }

        char buffer[128];
        switch (player->state) {
            case PLAYER_ON_FOOT:
                break;
            case PLAYER_PICK_UP:
                break;
            case PLAYER_SHOOT:
                draw_item_use(components, window, camera, i);
                break;
            case PLAYER_RELOAD:
                sfConvexShape_setFillColor(player->shape, sfWhite);
                int akimbo = get_akimbo(components, item);
                float prog = 2 * M_PI * (1.0 - weapon->cooldown / ((1 + akimbo) * weapon->reload_time));
                draw_slice(window, components, camera, NULL, 50, position, 0.75, 1.0, 0.5 * M_PI - 0.5 * prog, prog, sfWhite);

                break;
            case PLAYER_ENTER:
                break;
            case PLAYER_DRIVE:
                // VehicleComponent* vehicle = VehicleComponent_get(player->vehicle);
                // if (vehicle) {
                //     draw_slice(window, components, camera, NULL, 50, get_position(components, player->vehicle), 1.0, 1.2, 0.5 * M_PI, vehicle->fuel / vehicle->max_fuel * M_PI, sfWhite);
                // }

                break;
            case PLAYER_PASSENGER:
                break;
            case PLAYER_MENU:
            case PLAYER_MENU_DROP:
            case PLAYER_MENU_GRAB:
                snprintf(buffer, 128, "%d", player->money);
                draw_text(window, components, camera, NULL, position, buffer, 20, sfYellow);

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
                        
                    draw_menu_slot(components, window, camera, i, j, offset, alpha);

                    if (player->inventory[j] != -1) {
                        ItemComponent* item = components->item[player->inventory[j]];
                        for (int k = 0; k < item->size; k++) {
                            if (j == slot) {
                                offset = 0.2;
                                if (k == atch && item->attachments[k] == -1) {
                                    offset = 0.4;
                                }
                            }

                            draw_menu_attachment(components, window, camera, i, j, k, offset, alpha);
                        }
                    }
                }

                break;
            case PLAYER_AMMO_MENU:
                snprintf(buffer, 128, "%d", player->money);
                draw_text(window, components, camera, NULL, position, buffer, 20, sfYellow);
                
                draw_ammo_menu(components, window, camera, i);
                break;
            case PLAYER_DEAD:
                break;
        }
    }
}
