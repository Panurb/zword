#define _USE_MATH_DEFINES
#include <math.h>

#include "player.h"
#include "weapon.h"


void draw_menu_slot(ComponentData* components, sfRenderWindow* window, int camera, int entity, int slot, float offset, float alpha) {
    PlayerComponent* player = PlayerComponent_get(components, entity);
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
        ItemComponent* item = components->item[i];

        sfSprite* sprite = ImageComponent_get(components, i)->sprite;

        sfVector2f r = polar_to_cartesian(1.5 + offset, slot * slice - 0.5 * slice + 0.5 * slice / (item->size + 1));

        if (i == player->grabbed_item) {
            r = mult(2.5, player->controller.right_stick);
        }

        draw_sprite(window, components, camera, sprite, sum(pos, r), (slot - 1) * 0.5f * M_PI, ones(), 0);
    }
}


void draw_menu_attachment(ComponentData* components, sfRenderWindow* window, int camera, int entity, int slot, int atch, float offset, float alpha) {
    PlayerComponent* player = PlayerComponent_get(components, entity);
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

        sfSprite* sprite = ImageComponent_get(components, a)->sprite;

        sfVector2f r = polar_to_cartesian(1.5 + offset, slot * slice - 0.5 * slice + (atch + 1.5) * slice / (item->size + 1));

        if (a == player->grabbed_item) {
            r = mult(2.5, player->controller.right_stick);
        }

        draw_sprite(window, components, camera, sprite, sum(pos, r), (slot - 1) * 0.5f * M_PI, mult(0.75, ones()), 0);
    }
}


void draw_hud(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        PlayerComponent* player = PlayerComponent_get(components, i);
        if (!player) continue;

        sfVector2f pos = get_position(components, i);

        int slot = get_inventory_slot(components, i);
        int atch = get_attachment(components, i);

        int item = player->inventory[player->item];
        WeaponComponent* weapon = NULL;
        if (item != -1) {
            weapon = components->weapon[item];
        }

        switch (player->state) {
            case ON_FOOT:
            case SHOOT:
                if (weapon) {
                    CameraComponent* cam = CameraComponent_get(components, camera);

                    float spread = fmax(0.01, weapon->recoil);
                    // draw_cone(window, components, camera, NULL, 20, get_position(components, i), 3.0, get_angle(components, i), spread);
                }

                break;
            case RELOAD:
                sfConvexShape_setFillColor(player->shape, sfWhite);
                int akimbo = get_akimbo(components, item);
                float prog = 2 * M_PI * (1.0 - weapon->cooldown / ((1 + akimbo) * weapon->reload_time));
                draw_slice(window, components, camera, NULL, 50, pos, 0.75, 1.0, 0.5 * M_PI - 0.5 * prog, prog, sfWhite);

                break;
            case DRIVE:
                ;
                VehicleComponent* vehicle = VehicleComponent_get(components, player->vehicle);
                if (vehicle) {
                    draw_slice(window, components, camera, NULL, 50, get_position(components, player->vehicle), 1.0, 1.2, 0.5 * M_PI, vehicle->fuel / vehicle->max_fuel * M_PI, sfWhite);
                }

                break;
            case MENU:
            case MENU_DROP:
            case MENU_GRAB:
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
            case PLAYER_DEAD:
                break;
        }
    }
}
