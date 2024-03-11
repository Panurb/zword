#include <stdio.h>

#include "component.h"
#include "grid.h"
#include "util.h"
#include "image.h"
#include "weapon.h"
#include "game.h" 
#include "player.h"


int create_flashlight(sfVector2f position) {
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


int create_gas(sfVector2f position) {
    int i = create_entity();

    float angle = rand_angle();
    CoordinateComponent_add(i, position, angle);
    ColliderComponent_add_rectangle(i, 0.75, 0.8, GROUP_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    ImageComponent_add(i, "gas", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent_add(i, 0, 0, "");

    return i;
}


int create_bandage(sfVector2f position) {
    int i = create_entity();

    float angle = rand_angle();
    CoordinateComponent_add(i, position, angle);
    ColliderComponent_add_circle(i, 0.5f, GROUP_ITEMS);
    PhysicsComponent_add(i, 0.5f);
    ImageComponent_add(i, "bandage", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent* item = ItemComponent_add(i, 0, 0, "");
    item->use_time = 2.0f;
    item->value = 100;

    return i;
}


void create_item(sfVector2f position, int tier) {
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
                create_axe(game_data->components, position);
            } else {
                create_pistol(game_data->components, position);
                for (int i = 0; i < randi(1, 3); i++) {
                    create_ammo(game_data->components, sum(position, rand_vector()), AMMO_PISTOL);
                }
            }
            break;
        case 2:
            create_shotgun(game_data->components, position);
            for (int i = 0; i < randi(1, 3); i++) {
                    create_ammo(game_data->components, sum(position, rand_vector()), AMMO_SHOTGUN);
            }
            break;
        case 3:
            create_assault_rifle(game_data->components, position);
            for (int i = 0; i < randi(1, 3); i++) {
                create_ammo(game_data->components, sum(position, rand_vector()), AMMO_RIFLE);
            }
            break;
        default:
            create_ammo(game_data->components, position, randi(1, 3));
            break;
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

    CoordinateComponent* coord = CoordinateComponent_get(player->target);
    ImageComponent* image = ImageComponent_get(player->target);
    AmmoComponent* ammo = AmmoComponent_get(player->target);
    if (ammo) {
        clear_grid(player->target);

        int i = player->ammo[ammo->type];
        if (i == -1) {
            coord->parent = entity;
            coord->position = zeros();
            coord->angle = 0.0f;
            player->ammo[ammo->type] = player->target;
            image->alpha = 0.0f;
            ColliderComponent_get(player->target)->enabled = false;
        } else {
            AmmoComponent_get(i)->size += ammo->size;
            destroy_entity(player->target);
            player->target = -1;
        }
    } else {
        int i = find(-1, player->inventory, player->inventory_size);
        if (i != -1) {
            clear_grid(player->target);

            player->inventory[i] = player->target;
            coord->parent = entity;
            coord->position = (sfVector2f) { 0.75f, 0.0f };
            coord->angle = 0.0f;
            ColliderComponent_get(player->target)->enabled = false;
            change_layer(player->target, LAYER_WEAPONS);
            if (player->item != i) {
                image->alpha = 0.0f;
            }
            WeaponComponent* weapon = WeaponComponent_get(player->target);
            if (weapon) {
                image->alpha = 0.0f;
                if (item->price != 0) {
                    player->item = i;
                }
            }

            add_money(entity, -item->price);
            item->price = 0;
        }
    }
}


void drop_item(int entity) {
    PlayerComponent* player = PlayerComponent_get(entity);

    int i = player->inventory[player->item];
    if (i != -1) {
        PhysicsComponent* physics = PhysicsComponent_get(i);
        player->inventory[player->item] = -1;

        CoordinateComponent* coord = CoordinateComponent_get(i);
        coord->parent = -1;

        sfVector2f r = polar_to_cartesian(1.0, get_angle(entity));
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

    player_health->health = min(player_health->health + item->value, player_health->max_health);

    drop_item(parent);
    clear_grid(entity);
    destroy_entity(entity);
}


void switch_light(int entity) {
    LightComponent* light = LightComponent_get(entity);
    if (light) {
        light->enabled = true;
    }
}


void use_item(int entity, float time_step) {
    int parent = CoordinateComponent_get(entity)->parent;

    PlayerComponent* player = PlayerComponent_get(parent);
    ItemComponent* item = ItemComponent_get(entity);

    if (player->use_timer >= item->use_time) {
        switch (item->type) {
            case ITEM_HEAL:
                heal(entity);
                break;
            case ITEM_LIGHT:
                switch_light(entity);
                break;
            default:
                break;
        }
    } else {
        player->use_timer += time_step;
    }
}


void draw_items() {
    sfText* text = sfText_create();
    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(node->value);
        if (player->target == -1) continue;
        ItemComponent* item = ItemComponent_get(player->target);
        ImageComponent* image = ImageComponent_get(player->target);

        sfShader_setFloatUniform(CameraComponent_get(game_data->camera)->shaders[1], "offset", 0.05f);
        sfVector2f pos = get_position(player->target);
        float angle = get_angle(player->target);
        if (image->alpha != 0.0f) {
            draw_sprite(game_data->camera, image->sprite, pos, angle, image->scale, SHADER_OUTLINE);
        }

        if (item) {
            pos = sum(pos, vec(0.0f, 1.0f));
            char buffer[256];
            if (item->price > 0) {
                snprintf(buffer, 256, "%d", item->price);
                draw_text(game_data->camera, text, pos, buffer, 20, sfYellow);
                pos = sum(pos, vec(0.0f, 1.0f));
            }

            draw_text(game_data->camera, text, pos, item->name, 20, sfYellow);
        }
    }
    sfText_destroy(text);
}
