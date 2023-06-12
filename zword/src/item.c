#include "component.h"
#include "grid.h"
#include "util.h"
#include "image.h"
#include "weapon.h"


void create_flashlight(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    float angle = rand_angle();
    CoordinateComponent_add(components, i, position, angle);
    ColliderComponent_add_rectangle(components, i, 1.0, 0.5, GROUP_ITEMS);
    PhysicsComponent_add(components, i, 0.5f);
    ImageComponent_add(components, i, "flashlight", 1.0, 1.0, 3);
    ItemComponent_add(components, i, 0);
    LightComponent_add(components, i, 15.0f, 1.0, get_color(1.0, 1.0, 0.8, 1.0), 0.75, 10.0)->enabled = false;
}


void create_gas(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    float angle = rand_angle();
    CoordinateComponent_add(components, i, position, angle);
    ColliderComponent_add_rectangle(components, i, 0.75, 0.8, GROUP_ITEMS);
    PhysicsComponent_add(components, i, 0.5f);
    ImageComponent_add(components, i, "gas", 1.0, 1.0, 3);
    ItemComponent_add(components, i, 0);
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
            image->outline = 0.0f;
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
            image->outline = 0.0f;
            if (player->item != i) {
                image->alpha = 0.0f;
            }
            WeaponComponent* weapon = WeaponComponent_get(components, player->target);
            if (weapon && weapon->ammo_type != AMMO_MELEE) {
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
