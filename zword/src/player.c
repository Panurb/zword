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
#include "vehicle.h"
#include "item.h"


void create_player(ComponentData* components, sfVector2f pos) {
    int i = get_index(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    ImageComponent_add(components, i, "player", 1.0, 1.0, 5);
    components->physics[i] = PhysicsComponent_create(1.0, 0.0, 0.0, 10.0, 0.0);
    components->physics[i]->max_speed = 5.0;
    components->collider[i] = ColliderComponent_create_circle(0.5);
    components->player[i] = PlayerComponent_create();
    components->particle[i] = ParticleComponent_create(0.0, 2 * M_PI, 0.5, 0.0, 5.0, 10.0, sfColor_fromRGB(200, 0, 0), sfColor_fromRGB(255, 0, 0));
    components->waypoint[i] = WaypointComponent_create();
    components->health[i] = HealthComponent_create(100);
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


sfVector2f right_stick(ComponentData* component, sfRenderWindow* window, Camera* camera, int i) {
    sfVector2f mouse = screen_to_world(sfMouse_getPosition((sfWindow*) window), camera);
    sfVector2f rel_mouse = diff(mouse, get_position(component, i));

    return normalized(rel_mouse);
}


int get_inventory_slot(ComponentData* component, sfRenderWindow* window, Camera* camera, int i) {
    PlayerComponent* player = component->player[i];

    sfVector2f rs = right_stick(component, window, camera, i);
    float angle = mod(polar_angle(rs) + 0.25 * M_PI, 2 * M_PI);

    return floor(player->inventory_size * angle / (2 * M_PI));
}


int get_attachment(ComponentData* component, sfRenderWindow* window, Camera* camera, int i) {
    PlayerComponent* player = component->player[i];
    int slot = get_inventory_slot(component, window, camera, i);
    int item = player->inventory[slot];

    if (item != -1) {
        int size = component->item[item]->size;
        if (size > 0) {
            float slot_angle = 2 * M_PI / player->inventory_size;
            sfVector2f rs = right_stick(component, window, camera, i);
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
        }
    }
}


void update_players(ComponentData* component, ColliderGrid* grid, sfRenderWindow* window, Camera* camera, float time_step) {
    for (int i = 0; i < component->entities; i++) {
        PlayerComponent* player = component->player[i];
        if (!player) continue;

        PhysicsComponent* phys = component->physics[i];
        CoordinateComponent* coord = component->coordinate[i];
        int item = player->inventory[player->item];
        sfVector2f v = left_stick();
        int slot = get_inventory_slot(component, window, camera, i);
        int atch = get_attachment(component, window, camera, i);

        WeaponComponent* weapon = NULL;
        if (item != -1) {
            weapon = component->weapon[item];

            if (weapon) {
                weapon->cooldown = fmax(0.0, weapon->cooldown - time_step);
                weapon->recoil = fmax(0.1 * norm(phys->velocity), weapon->recoil - time_step * weapon->recoil_down);
            }
        }
        
        switch (player->state) {
            case ON_FOOT:
                phys->acceleration = sum(phys->acceleration, mult(player->acceleration, v));
                coord->angle = polar_angle(right_stick(component, window, camera, i));

                break;
            case SHOOT:
                phys->acceleration = sum(phys->acceleration, mult(player->acceleration, v));
                coord->angle = polar_angle(right_stick(component, window, camera, i));

                if (weapon) {
                    shoot(component, grid, item);

                    if (weapon->reloading) {
                        player->state = RELOAD;
                    }
                }

                break;
            case RELOAD:
                phys->acceleration = sum(phys->acceleration, mult(player->acceleration, v));
                coord->angle = polar_angle(right_stick(component, window, camera, i));

                reload(component, item);

                if (weapon->cooldown == 0.0) {
                    weapon->magazine = weapon->max_magazine;
                    weapon->reloading = false;

                    player->state = ON_FOOT;
                }

                break;
            case DRIVE:
                drive_vehicle(component, i, v, time_step);

                break;
            case MENU:
                if (item != -1) {
                    if (component->light[item]) {
                        component->light[item]->enabled = false;
                    }
                    if (component->weapon[item]) {
                        component->weapon[item]->reloading = false;
                    }

                    for (int j = 0; j < component->item[item]->size; j++) {
                        int a = component->item[item]->attachments[j];
                        if (component->light[a]) {
                            component->light[a]->enabled = false;
                        }
                    }
                }

                player->item = get_inventory_slot(component, window, camera, i);

                item = player->inventory[player->item];

                if (item != -1) {
                    if (component->light[item]) {
                        component->light[item]->enabled = true;
                    }

                    for (int j = 0; j < component->item[item]->size; j++) {
                        int a = component->item[item]->attachments[j];
                        if (component->light[a]) {
                            component->light[a]->enabled = true;
                        }
                    }
                }

                break;
            case MENU_GRAB:
                if (player->grabbed_item == -1 && player->inventory[slot] != -1) {
                    if (atch == -1) {
                        player->grabbed_item = item;
                    } else if (component->item[item]->attachments[atch] != -1) {
                        player->grabbed_item = component->item[item]->attachments[atch];
                    }
                }

                break;
            case MENU_DROP:
                if (player->grabbed_item != -1) {
                    if (player->inventory[slot] == -1) {
                        replace(player->grabbed_item, -1, player->inventory, player->inventory_size);
                        for (int j = 0; j < player->inventory_size; j++) {
                            if (player->inventory[j] == -1) continue;
                            ItemComponent* it = component->item[player->inventory[j]];
                            replace(player->grabbed_item, -1, it->attachments, it->size);
                        }
                        player->inventory[slot] = player->grabbed_item;
                        component->coordinate[player->grabbed_item]->parent = i;
                    } else if (atch != -1) {
                        if (player->inventory[slot] != player->grabbed_item && component->item[player->inventory[slot]]->attachments[atch] == -1) {
                            replace(player->grabbed_item, -1, player->inventory, player->inventory_size);
                            for (int j = 0; j < player->inventory_size; j++) {
                                if (player->inventory[j] == -1) continue;
                                ItemComponent* it = component->item[player->inventory[j]];
                                replace(player->grabbed_item, -1, it->attachments, it->size);
                            }
                            component->item[player->inventory[slot]]->attachments[atch] = player->grabbed_item;
                            component->coordinate[player->grabbed_item]->parent = player->inventory[slot];
                        }
                    }
                    player->grabbed_item = -1;
                }

                break;
        }
    }
}


void draw_menu_slot(ComponentData* components, sfRenderWindow* window, Camera* camera, int entity, int slot, float offset, float alpha) {
    PlayerComponent* player = components->player[entity];
    sfVector2f pos = get_position(components, entity);

    float gap = 0.2;
    float slice = (2 * M_PI / player->inventory_size);

    sfColor color = sfConvexShape_getFillColor(player->shape);
    color.a = alpha * 255;
    sfConvexShape_setFillColor(player->shape, color);

    if (alpha == 0.0) {
        draw_slice_outline(window, camera, player->line, pos, 1.0 + offset, 2.0 + offset, slot * slice, slice - gap);
    } else {
        draw_slice(window, camera, player->shape, pos, 1.0 + offset, 2.0 + offset, slot * slice, slice - gap);
    }

    int i = player->inventory[slot];
    if (i != -1) {
        ItemComponent* item = components->item[i];

        sfSprite* sprite = ImageComponent_get(components, i)->sprite;

        sfVector2f r = polar_to_cartesian(1.5 + offset, slot * slice - 0.5 * slice + 0.5 * slice / (item->size + 1));
        draw_sprite(window, camera, sprite, sum(pos, r), 0.0);
    }
}


void draw_menu_attachment(ComponentData* components, sfRenderWindow* window, Camera* camera, int entity, int slot, int atch, float offset, float alpha) {
    PlayerComponent* player = components->player[entity];
    sfVector2f pos = get_position(components, entity);
    ItemComponent* item = components->item[player->inventory[slot]];

    float gap = 0.2;
    float slice = (2 * M_PI / player->inventory_size);

    float spread = (slice - 2 * gap - 0.5 * item->size * gap) / (item->size + 1);
    float angle = (slot - 0.5) * slice + (atch + 1.5) * spread + (0.5 * atch + 1.5) * gap;
    draw_slice_outline(window, camera, player->line, pos, 1.2 + offset, 1.8 + offset, angle, spread);

    if (item->attachments[atch] != -1) {
        sfColor color = sfConvexShape_getFillColor(player->shape);
        color.a = alpha * 255;
        sfConvexShape_setFillColor(player->shape, color);

        draw_slice(window, camera, player->shape, pos, 1.2 + offset, 1.8 + offset, angle, spread);

        int a = item->attachments[atch];

        sfSprite* sprite = ImageComponent_get(components, a)->sprite;

        sfVector2f r = polar_to_cartesian(1.5 + offset, slot * slice - 0.5 * slice + (atch + 1.5) * slice / (item->size + 1));
        draw_sprite(window, camera, sprite, sum(pos, r), 0.0);
    }
}


void draw_players(ComponentData* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        PlayerComponent* player = component->player[i];
        if (!player) continue;

        sfVector2f pos = get_position(component, i);

        int slot = get_inventory_slot(component, window, camera, i);
        int atch = get_attachment(component, window, camera, i);

        int item = player->inventory[player->item];
        WeaponComponent* weapon = NULL;
        if (item != -1) {
            weapon = component->weapon[item];
        }

        switch (player->state) {
            case ON_FOOT:
            case SHOOT:
                if (weapon) {
                    sfConvexShape_setOutlineThickness(weapon->shape, 0.02 * camera->zoom);

                    float spread = fmax(0.01, weapon->recoil);
                    draw_cone(window, camera, weapon->shape, 20, get_position(component, i), 3.0, get_angle(component, i), spread);
                }

                break;
            case RELOAD:
                sfConvexShape_setFillColor(player->shape, sfWhite);
                float prog = 2 * M_PI * (1 - weapon->cooldown / weapon->reload_time);
                draw_slice(window, camera, player->shape, pos, 0.75, 1.0, 0.5 * M_PI - 0.5 * prog, prog);

                break;
            case DRIVE:

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

                    draw_menu_slot(component, window, camera, i, j, offset, alpha);

                    if (player->inventory[j] != -1) {
                        ItemComponent* item = component->item[player->inventory[j]];
                        for (int k = 0; k < item->size; k++) {
                            if (j == slot) {
                                offset = 0.2;
                                if (k == atch && item->attachments[k] != -1) {
                                    offset = 0.4;
                                }
                            }

                            draw_menu_attachment(component, window, camera, i, j, k, offset, alpha);
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
                        
                    draw_menu_slot(component, window, camera, i, j, offset, alpha);

                    if (player->inventory[j] != -1) {
                        ItemComponent* item = component->item[player->inventory[j]];
                        for (int k = 0; k < item->size; k++) {
                            if (j == slot) {
                                offset = 0.2;
                                if (k == atch && item->attachments[k] == -1) {
                                    offset = 0.4;
                                }
                            }

                            draw_menu_attachment(component, window, camera, i, j, k, offset, alpha);
                        }
                    }
                }

                break;
        }
    }
}
