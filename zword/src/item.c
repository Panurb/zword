#include <stdio.h>

#include "component.h"
#include "grid.h"
#include "util.h"
#include "image.h"
#include "weapon.h"
#include "game.h" 
#include "player.h"


int create_flashlight(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    float angle = rand_angle();
    CoordinateComponent_add(components, i, position, angle);
    ColliderComponent_add_rectangle(components, i, 1.0, 0.5, GROUP_ITEMS);
    PhysicsComponent_add(components, i, 0.5f);
    ImageComponent_add(components, i, "flashlight", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent_add(components, i, 0, 100, "Flashlight");
    LightComponent_add(components, i, 15.0f, 1.0, get_color(1.0, 1.0, 0.8, 1.0), 0.75, 10.0)->enabled = false;

    return i;
}


int create_gas(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    float angle = rand_angle();
    CoordinateComponent_add(components, i, position, angle);
    ColliderComponent_add_rectangle(components, i, 0.75, 0.8, GROUP_ITEMS);
    PhysicsComponent_add(components, i, 0.5f);
    ImageComponent_add(components, i, "gas", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent_add(components, i, 0, 0, "");

    return i;
}


int create_bandage(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    float angle = rand_angle();
    CoordinateComponent_add(components, i, position, angle);
    ColliderComponent_add_circle(components, i, 0.5f, GROUP_ITEMS);
    PhysicsComponent_add(components, i, 0.5f);
    ImageComponent_add(components, i, "bandage", 1.0, 1.0, LAYER_ITEMS);
    ItemComponent* item = ItemComponent_add(components, i, 0, 0, "");
    item->use_time = 2.0f;
    item->value = 100;

    return i;
}


void create_item(ComponentData* components, sfVector2f position, int tier) {
    switch (tier) {
        case 0:
            if (randi(0, 1) == 0) {
                create_flashlight(components, position);
            } else {
                create_gas(components, position);
            }
            break;
        case 1:
            if (randi(0, 1) == 0) {
                create_axe(components, position);
            } else {
                create_pistol(components, position);
                for (int i = 0; i < randi(1, 3); i++) {
                    create_ammo(components, sum(position, rand_vector()), AMMO_PISTOL);
                }
            }
            break;
        case 2:
            create_shotgun(components, position);
            for (int i = 0; i < randi(1, 3); i++) {
                    create_ammo(components, sum(position, rand_vector()), AMMO_SHOTGUN);
            }
            break;
        case 3:
            create_assault_rifle(components, position);
            for (int i = 0; i < randi(1, 3); i++) {
                create_ammo(components, sum(position, rand_vector()), AMMO_RIFLE);
            }
            break;
        default:
            create_ammo(components, position, randi(1, 3));
            break;
    }
}


void pick_up_item(ComponentData* components, ColliderGrid* grid, int entity) {
    PlayerComponent* player = PlayerComponent_get(components, entity);

    if (player->target == -1) {
        return;
    }
    
    ItemComponent* item = ItemComponent_get(components, player->target);
    if (item->price > player->money) {
        return;
    }
    add_money(components, entity, -item->price);
    item->price = 0;

    clear_grid(components, grid, player->target);

    CoordinateComponent* coord = CoordinateComponent_get(components, player->target);
    ImageComponent* image = ImageComponent_get(components, player->target);
    AmmoComponent* ammo = AmmoComponent_get(components, player->target);
    if (ammo) {
        int i = player->ammo[ammo->type];
        if (i == -1) {
            coord->parent = entity;
            coord->position = zeros();
            coord->angle = 0.0f;
            player->ammo[ammo->type] = player->target;
            image->alpha = 0.0f;
            ColliderComponent_get(components, player->target)->enabled = false;
        } else {
            AmmoComponent_get(components, i)->size += ammo->size;
            destroy_entity(components, player->target);
            player->target = -1;
        }
    } else {
        int i = find(-1, player->inventory, player->inventory_size);
        if (i != -1) {
            player->inventory[i] = player->target;
            coord->parent = entity;
            coord->position = (sfVector2f) { 0.75f, 0.0f };
            coord->angle = 0.0f;
            ColliderComponent_get(components, player->target)->enabled = false;
            change_layer(components, player->target, LAYER_WEAPONS);
            if (player->item != i) {
                image->alpha = 0.0f;
            }
            WeaponComponent* weapon = WeaponComponent_get(components, player->target);
            if (weapon) {
                image->alpha = 0.0f;
            }
        }
    }
}


void drop_item(ComponentData* components, int entity) {
    PlayerComponent* player = PlayerComponent_get(components, entity);

    int i = player->inventory[player->item];
    if (i != -1) {
        PhysicsComponent* physics = PhysicsComponent_get(components, i);
        player->inventory[player->item] = -1;

        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        coord->parent = -1;

        sfVector2f r = polar_to_cartesian(1.0, get_angle(components, entity));
        coord->position = sum(get_position(components, entity), r);

        physics->velocity = mult(7.0, r);
        physics->angular_velocity = 3.0;

        LightComponent* light = LightComponent_get(components, entity);
        if (light) {
            light->enabled = false;
        }

        change_layer(components, i, LAYER_ITEMS);
        ImageComponent_get(components, i)->alpha = 1.0f;
        ColliderComponent_get(components, i)->enabled = true;
    }
}


void heal(ComponentData* components, ColliderGrid* grid, int entity) {
    int parent = CoordinateComponent_get(components, entity)->parent;

    ItemComponent* item = ItemComponent_get(components, entity);

    HealthComponent* player_health = HealthComponent_get(components, parent);

    player_health->health = min(player_health->health + item->value, player_health->max_health);

    drop_item(components, parent);
    clear_grid(components, grid, entity);
    destroy_entity(components, entity);
}


void switch_light(ComponentData* components, int entity) {
    LightComponent* light = LightComponent_get(components, entity);
    if (light) {
        light->enabled = true;
    }
}


void use_item(ComponentData* components, ColliderGrid* grid, int entity, float time_step) {
    int parent = CoordinateComponent_get(components, entity)->parent;

    PlayerComponent* player = PlayerComponent_get(components, parent);
    ItemComponent* item = ItemComponent_get(components, entity);

    if (player->use_timer >= item->use_time) {
        switch (item->type) {
            case ITEM_HEAL:
                heal(components, grid, entity);
                break;
            case ITEM_LIGHT:
                switch_light(components, entity);
                break;
            default:
                break;
        }
    } else {
        player->use_timer += time_step;
    }
}


void draw_items(GameData* data, sfRenderWindow* window) {
    sfText* text = sfText_create();
    ListNode* node;
    FOREACH(node, data->components->player.order) {
        PlayerComponent* player = PlayerComponent_get(data->components, node->value);
        if (player->target == -1) continue;
        ItemComponent* item = ItemComponent_get(data->components, player->target);
        ImageComponent* image = ImageComponent_get(data->components, player->target);

        sfShader_setFloatUniform(CameraComponent_get(data->components, data->camera)->shaders[1], "offset", 0.05f);
        sfVector2f pos = get_position(data->components, player->target);
        float angle = get_angle(data->components, player->target);
        if (image->alpha != 0.0f) {
            draw_sprite(window, data->components, data->camera, image->sprite, pos, angle, image->scale, SHADER_OUTLINE);
        }

        if (item) {
            pos = sum(pos, vec(0.0f, 1.0f));
            char buffer[256];
            if (item->price > 0) {
                snprintf(buffer, 256, "%d", item->price);
                draw_text(window, data->components, data->camera, text, pos, buffer, 20, sfYellow);
                pos = sum(pos, vec(0.0f, 1.0f));
            }

            draw_text(window, data->components, data->camera, text, pos, item->name, 20, sfYellow);
        }
    }
    sfText_destroy(text);
}
