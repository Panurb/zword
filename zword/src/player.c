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


void create_player(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    ImageComponent_add(components, i, "player", 1.0, 1.0, 5);
    PhysicsComponent_add(components, i, 1.0, 0.0, 0.0, 10.0, 0.0)->max_speed = 5.0;
    ColliderComponent_add_circle(components, i, 0.5, PLAYERS);
    PlayerComponent_add(components, i);
    ParticleComponent_add(components, i, 0.0, 2 * M_PI, 0.5, 0.0, 5.0, 10.0, get_color(0.78, 0.0, 0.0, 1.0), sfRed);
    WaypointComponent_add(components, i);
    HealthComponent_add(components, i, 100);
    SoundComponent_add(components, i, "squish");
}


sfVector2f left_stick() {
    sfVector2f v = zeros();

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


sfVector2f right_stick(ComponentData* components, sfRenderWindow* window, int camera, int i) {
    sfVector2f mouse = screen_to_world(components, camera, sfMouse_getPosition((sfWindow*) window));
    sfVector2f rel_mouse = diff(mouse, get_position(components, i));

    return normalized(rel_mouse);
}


int get_inventory_slot(ComponentData* components, sfRenderWindow* window, int camera, int i) {
    PlayerComponent* player = components->player[i];

    sfVector2f rs = right_stick(components, window, camera, i);
    float angle = mod(polar_angle(rs) + 0.25 * M_PI, 2 * M_PI);

    return floor(player->inventory_size * angle / (2 * M_PI));
}


int get_attachment(ComponentData* components, sfRenderWindow* window, int camera, int i) {
    PlayerComponent* player = components->player[i];
    int slot = get_inventory_slot(components, window, camera, i);
    int item = player->inventory[slot];

    if (item != -1) {
        int size = components->item[item]->size;
        if (size > 0) {
            float slot_angle = 2 * M_PI / player->inventory_size;
            sfVector2f rs = right_stick(components, window, camera, i);
            float angle = polar_angle(rs) + 0.25 * M_PI;
            angle = mod(angle, slot_angle);

            return floor((size + 1) * angle / slot_angle) - 1;
        }
    }

    return -1;
}


void input(ComponentData* component) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->player[i]) continue;
        
        PlayerComponent* player = component->player[i];

        switch (player->state) {
            case ON_FOOT:
                if (sfKeyboard_isKeyPressed(sfKeySpace)) {
                    player->state = MENU;
                }

                if (sfMouse_isButtonPressed(sfMouseRight)) {
                    pick_up_item(component, i);
                }

                if (sfMouse_isButtonPressed(sfMouseLeft)) {
                    player->state = SHOOT;
                }

                if (sfKeyboard_isKeyPressed(sfKeyR)) {
                    if (component->weapon[player->inventory[player->item]]) {
                        player->state = RELOAD;
                    }
                }

                if (sfKeyboard_isKeyPressed(sfKeyF)) {
                    if (enter_vehicle(component, i)) {
                        player->state = DRIVE;
                    }
                }

                break;
            case SHOOT:
                if (!sfMouse_isButtonPressed(sfMouseLeft)) {
                    player->state = ON_FOOT;
                }

                break;
            case RELOAD:
                if (sfKeyboard_isKeyPressed(sfKeySpace)) {
                    player->state = MENU;
                }

                break;
            case DRIVE:
                if (sfKeyboard_isKeyPressed(sfKeyE)) {
                    exit_vehicle(component, i);

                    player->state = ON_FOOT;
                }

                break;
            case MENU:
                if (sfMouse_isButtonPressed(sfMouseLeft)) {
                    player->state = MENU_GRAB;
                }

                if (!sfKeyboard_isKeyPressed(sfKeySpace)) {
                    player->state = ON_FOOT;
                }

                if (sfMouse_isButtonPressed(sfMouseRight)) {
                    drop_item(component, i);
                }

                break;
            case MENU_GRAB:
                if (!sfMouse_isButtonPressed(sfMouseLeft)) {
                    player->state = MENU_DROP;
                }

                break;
            case MENU_DROP:
                if (sfKeyboard_isKeyPressed(sfKeySpace)) {
                    player->state = MENU;
                } else {
                    player->state = ON_FOOT;
                }

                break;
            case PLAYER_DEAD:
                break;
        }
    }
}


void update_players(ComponentData* components, ColliderGrid* grid, sfRenderWindow* window, int camera, float time_step) {
    for (int i = 0; i < components->entities; i++) {
        PlayerComponent* player = components->player[i];
        if (!player) continue;

        PhysicsComponent* phys = components->physics[i];
        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        sfVector2f v = left_stick();
        int slot = get_inventory_slot(components, window, camera, i);
        int atch = get_attachment(components, window, camera, i);

        int item = player->inventory[player->item];
        WeaponComponent* weapon = NULL;
        if (item != -1) {
            weapon = components->weapon[item];
        }

        ImageComponent* image = ImageComponent_get(components, i);

        if (HealthComponent_get(components, i)->health <= 0) {
            player->state = PLAYER_DEAD;
        }
        
        switch (player->state) {
            case ON_FOOT:
                phys->acceleration = sum(phys->acceleration, mult(player->acceleration, v));
                coord->angle = polar_angle(right_stick(components, window, camera, i));

                sfVector2f pos = get_position(components, i);
                int entities[100];
                get_entities(components, grid, pos, 1.0, entities);

                if (player->target != -1) {
                    ImageComponent_get(components, player->target)->outline = 0.0;
                }
                player->target = -1;

                float min_dist = 9999.0;
                for (int j = 0; j < 100; j++) {
                    int k = entities[j];
                    if (k == -1) break;
                    if (!ItemComponent_get(components, k)) continue;
                    if (CoordinateComponent_get(components, k)->parent != -1) continue;

                    float d = dist(pos, get_position(components, k));
                    if (d < min_dist) {
                        player->target = k;
                        min_dist = d;
                    }
                }

                if (player->target != -1) {
                    ImageComponent_get(components, player->target)->outline = 0.2;
                }

                break;
            case SHOOT:
                phys->acceleration = sum(phys->acceleration, mult(player->acceleration, v));
                coord->angle = polar_angle(right_stick(components, window, camera, i));

                if (weapon) {
                    shoot(components, grid, item);

                    if (weapon->reloading) {
                        player->state = RELOAD;
                    }
                }

                break;
            case RELOAD:
                phys->acceleration = sum(phys->acceleration, mult(player->acceleration, v));
                coord->angle = polar_angle(right_stick(components, window, camera, i));

                if (weapon) {
                    reload(components, item);

                    if (!weapon->reloading) {
                        player->state = ON_FOOT;
                    }
                }

                break;
            case DRIVE:
                drive_vehicle(components, i, v, time_step);

                break;
            case MENU:
                phys->acceleration = sum(phys->acceleration, mult(player->acceleration, v));

                if (item != -1) {
                    if (components->light[item]) {
                        components->light[item]->enabled = false;
                    }
                    if (components->weapon[item]) {
                        components->weapon[item]->reloading = false;
                    }

                    for (int j = 0; j < components->item[item]->size; j++) {
                        int a = components->item[item]->attachments[j];
                        if (components->light[a]) {
                            components->light[a]->enabled = false;
                        }
                    }
                }

                player->item = get_inventory_slot(components, window, camera, i);

                item = player->inventory[player->item];

                if (item != -1) {
                    if (components->light[item]) {
                        components->light[item]->enabled = true;
                    }

                    for (int j = 0; j < components->item[item]->size; j++) {
                        int a = components->item[item]->attachments[j];
                        if (components->light[a]) {
                            components->light[a]->enabled = true;
                        }
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
            case PLAYER_DEAD:
                strcpy(image->filename, "player_dead");
                image->width = 2.0;
                image->texture_changed = true;
                change_layer(components, i, 2);

                break;
        }
    }
}


void draw_menu_slot(ComponentData* components, sfRenderWindow* window, int camera, int entity, int slot, float offset, float alpha) {
    PlayerComponent* player = components->player[entity];
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
            r = mult(2.5, right_stick(components, window, camera, entity));
        }

        draw_sprite(window, components, camera, sprite, sum(pos, r), 0.0, ones(), 0);
    }
}


void draw_menu_attachment(ComponentData* components, sfRenderWindow* window, int camera, int entity, int slot, int atch, float offset, float alpha) {
    PlayerComponent* player = components->player[entity];
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
            r = mult(2.5, right_stick(components, window, camera, entity));
        }

        draw_sprite(window, components, camera, sprite, sum(pos, r), 0.0, mult(0.75, ones()), 0);
    }
}


void draw_players(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int i = 0; i < components->entities; i++) {
        PlayerComponent* player = components->player[i];
        if (!player) continue;

        sfVector2f pos = get_position(components, i);

        int slot = get_inventory_slot(components, window, camera, i);
        int atch = get_attachment(components, window, camera, i);

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
                    sfConvexShape_setOutlineThickness(weapon->shape, 0.02 * cam->zoom);

                    float spread = fmax(0.01, weapon->recoil);
                    draw_cone(window, components, camera, weapon->shape, 20, get_position(components, i), 3.0, get_angle(components, i), spread);
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
                for (int j = 0; j < player->inventory_size; j++) {
                    float offset = 0.0;
                    float alpha = 0.5;

                    if (j == slot) {
                        offset = 0.2;
                    }
                    
                    if (player->inventory[j] == -1) {
                        alpha = 0.25;
                    }

                    draw_menu_slot(components, window, camera, i, j, offset, alpha);

                    if (player->inventory[j] != -1) {
                        ItemComponent* item = components->item[player->inventory[j]];
                        for (int k = 0; k < item->size; k++) {
                            if (j == slot) {
                                offset = 0.2;
                                if (k == atch && item->attachments[k] != -1) {
                                    offset = 0.4;
                                }
                            }

                            draw_menu_attachment(components, window, camera, i, j, k, offset, alpha);
                        }
                    }
                }

                break;
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
