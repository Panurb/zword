#include <stdio.h>

#include "component.h"
#include "grid.h"
#include "util.h"
#include "image.h"
#include "weapon.h"
#include "game.h" 
#include "player.h"
#include "door.h"


bool is_attachment(int entity) {
    ItemComponent* item = ItemComponent_get(entity);
    if (!item) return false;
    
    switch (item->type) {
        case ITEM_LIGHT:
        case ITEM_LASER:
        case ITEM_SILENCER:
        case ITEM_SCOPE:
        case ITEM_MAGAZINE:
        case ITEM_HOLLOW_POINT:
        case ITEM_FUEL:
            return true;
        default:
            return false;
    }
}


int create_flashlight(Vector2f position) {
    int i = create_entity();

    float angle = rand_angle();
    CoordinateComponent_add(i, position, angle);
    ColliderComponent_add_rectangle(i, 1.0, 0.5, GROUP_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    ImageComponent_add(i, "flashlight", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent_add(i, 0, 100, "Flashlight");
    LightComponent_add(i, 15.0f, 1.0, get_color(1.0, 1.0, 0.8, 1.0), 0.75, 10.0)->enabled = false;

    return i;
}


int create_gas(Vector2f position) {
    int i = create_entity();

    float angle = rand_angle();
    CoordinateComponent_add(i, position, angle);
    ColliderComponent_add_rectangle(i, 0.75, 0.8, GROUP_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    ImageComponent_add(i, "gas", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent_add(i, 0, 0, "");

    return i;
}


int create_bandage(Vector2f position) {
    int i = create_entity();

    float angle = rand_angle();
    CoordinateComponent_add(i, position, angle);
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    ImageComponent_add(i, "bandage", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent* item = ItemComponent_add(i, 0, 0, "");
    item->use_time = 2.0f;
    item->value = 100;
    item->type = ITEM_HEAL;

    return i;
}


void create_item(Vector2f position, int tier) {
    switch (tier) {
        case 0:
            if (randi(0, 1) == 0) {
                create_flashlight(position);
            } else {
                create_gas(position);
            }
            break;
        case 1:
            if (randi(0, 1) == 0) {
                create_axe(position);
            } else {
                create_pistol(position);
                for (int i = 0; i < randi(1, 3); i++) {
                    create_ammo(sum(position, rand_vector()), AMMO_PISTOL);
                }
            }
            break;
        case 2:
            create_shotgun(position);
            for (int i = 0; i < randi(1, 3); i++) {
                    create_ammo(sum(position, rand_vector()), AMMO_SHOTGUN);
            }
            break;
        case 3:
            create_assault_rifle(position);
            for (int i = 0; i < randi(1, 3); i++) {
                create_ammo(sum(position, rand_vector()), AMMO_RIFLE);
            }
            break;
        default:
            create_ammo(position, randi(1, 3));
            break;
    }
}


void add_item_to_inventory(Entity player_entity, Entity item_entity) {
    PlayerComponent* player = PlayerComponent_get(player_entity);

    CoordinateComponent* coord = CoordinateComponent_get(item_entity);
    ImageComponent* image = ImageComponent_get(item_entity);
    ItemComponent* item = ItemComponent_get(item_entity);
    AmmoComponent* ammo = AmmoComponent_get(item_entity);

    if (ammo) {
        clear_grid(item_entity);

        int i = player->ammo[ammo->type];
        if (i == -1) {
            coord->parent = player_entity;
            coord->position = zeros();
            coord->angle = 0.0f;
            player->ammo[ammo->type] = item_entity;
            image->alpha = 0.0f;
            ColliderComponent_get(item_entity)->enabled = false;
        } else {
            AmmoComponent_get(i)->size += ammo->size;
            destroy_entity(item_entity);
            player->target = -1;
        }

        return;
    }

    if (item->type == ITEM_KEY) {
        int i = find(NULL_ENTITY, player->keys, player->keys_size);
        if (i != -1) {
            player->keys[i] = item_entity;
            coord->position = zeros();
            coord->angle = 0.0f;
            add_child(player_entity, item_entity);
        }
        return;
    }

    int i = find(-1, player->inventory, player->inventory_size);
    if (i != -1) {
        clear_grid(item_entity);

        player->inventory[i] = item_entity;
        coord->parent = player_entity;
        coord->position = (Vector2f) { 0.75f, 0.0f };
        coord->angle = 0.0f;
        ColliderComponent_get(item_entity)->enabled = false;
        change_layer(item_entity, LAYER_WEAPONS);
        if (player->item != i) {
            image->alpha = 0.0f;
        }
        WeaponComponent* weapon = WeaponComponent_get(item_entity);
        if (weapon) {
            image->alpha = 0.0f;
            if (item->price != 0) {
                player->item = i;
            }
        }

        add_money(player_entity, -item->price);
        item->price = 0;
    }
}


void pick_up_item(int entity) {
    PlayerComponent* player = PlayerComponent_get(entity);

    if (player->target == -1) {
        return;
    }
    
    ItemComponent* item = ItemComponent_get(player->target);
    if (item->price > player->money) {
        return;
    }

    add_item_to_inventory(entity, player->target);
}


void drop_item(int entity) {
    PlayerComponent* player = PlayerComponent_get(entity);

    int i = player->inventory[player->item];
    if (i != -1) {
        PhysicsComponent* physics = PhysicsComponent_get(i);
        player->inventory[player->item] = -1;

        CoordinateComponent* coord = CoordinateComponent_get(i);
        coord->parent = -1;

        Vector2f r = polar_to_cartesian(1.0, get_angle(entity));
        coord->position = sum(get_position(entity), r);

        physics->velocity = mult(7.0, r);
        physics->angular_velocity = 3.0;

        LightComponent* light = LightComponent_get(entity);
        if (light) {
            light->enabled = false;
        }

        change_layer(i, LAYER_ITEMS);
        ImageComponent_get(i)->alpha = 1.0f;
        ColliderComponent_get(i)->enabled = true;
    }
}


void heal(int entity) {
    int parent = CoordinateComponent_get(entity)->parent;

    ItemComponent* item = ItemComponent_get(entity);

    HealthComponent* player_health = HealthComponent_get(parent);

    player_health->health = mini(player_health->health + item->value, player_health->max_health);

    drop_item(parent);
    clear_grid(entity);
    destroy_entity(entity);
}


void switch_light(int entity) {
    LightComponent* light = LightComponent_get(entity);
    if (light) {
        light->enabled = !light->enabled;
    }
}


bool use_item(int entity, float time_step) {
    int parent = CoordinateComponent_get(entity)->parent;

    PlayerComponent* player = PlayerComponent_get(parent);
    ItemComponent* item = ItemComponent_get(entity);

    if (player->use_timer >= item->use_time) {
        switch (item->type) {
            case ITEM_HEAL:
                heal(entity);
                return true;
            case ITEM_LIGHT:
            case ITEM_LASER:
                switch_light(entity);
                return true;
            default:
                break;
        }
    } else {
        player->use_timer += time_step;
    }

    return false;
}


void draw_player_targets() {
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(node->value);
        if (player->target == -1) continue;
        ItemComponent* item = ItemComponent_get(player->target);
        ImageComponent* image = ImageComponent_get(player->target);
        DoorComponent* door = DoorComponent_get(player->target);

        if (door && door->price == 0) {
            continue;
        }

        Vector2f pos = get_position(player->target);
        float angle = get_angle(player->target);
        if (image->alpha != 0.0f) {
            draw_sprite_outline(game_data->camera, image->texture_index, image->width, image->height, 0, pos, angle, 
                get_scale(player->target));
        }

        if (item) {
            pos = sum(pos, vec(0.0f, 1.0f));
            char buffer[256];
            if (item->price > 0) {
                snprintf(buffer, 256, "%d", item->price);
                draw_text(game_data->camera, pos, buffer, 20, COLOR_YELLOW);
                pos = sum(pos, vec(0.0f, 1.0f));
            }

            draw_text(game_data->camera, pos, item->name, 20, COLOR_YELLOW);
        }

        if (door) {
            pos = sum(pos, vec(0.0f, 1.0f));
            String buffer = "";
            if (door->key != KEY_NONE) {
                snprintf(buffer, LENGTH(buffer), "Locked with %s", DOOR_KEY_NAMES[door->key]);
                draw_text(game_data->camera, pos, buffer, 20, COLOR_YELLOW);
                pos = sum(pos, vec(0.0f, 1.0f));
            } else if (game_data->game_mode == MODE_SURVIVAL && door->price > 0) {
                snprintf(buffer, LENGTH(buffer), "Locked, price %d", door->price);
                draw_text(game_data->camera, pos, buffer, 20, COLOR_YELLOW);
            }
        }
    }
}
