#include "component.h"
#include "grid.h"
#include "util.h"
#include "image.h"


void create_flashlight(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 1.0, 0.5, GROUP_ITEMS);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    ItemComponent_add(components, i, 0);
    LightComponent_add(components, i, 15.0f, 1.0, get_color(1.0, 1.0, 0.8, 1.0), 0.75, 10.0)->enabled = false;
    ImageComponent_add(components, i, "flashlight", 1.0, 1.0, 3);
}


void create_gas(ComponentData* components, sfVector2f position) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, position, rand_angle());
    ColliderComponent_add_rectangle(components, i, 0.75, 0.8, GROUP_ITEMS);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 1.0, 2.5);
    ItemComponent_add(components, i, 0);
    ImageComponent_add(components, i, "gas", 1.0, 1.0, 3);
}


void pick_up_item(ComponentData* components, ColliderGrid* grid, int entity) {
    PlayerComponent* player = PlayerComponent_get(components, entity);

    if (player->target == -1) {
        return;
    }

    AmmoComponent* ammo = AmmoComponent_get(components, player->target);
    if (ammo) {
        int i = player->ammo[ammo->type];
        if (i == -1) {
            CoordinateComponent* coord = CoordinateComponent_get(components, player->target);
            coord->parent = entity;
            coord->position = zeros();
            coord->angle = 0.0f;
            player->ammo[ammo->type] = player->target;
            ImageComponent_get(components, player->target)->alpha = 0.0f;
            ImageComponent_get(components, player->target)->outline = 0.0f;
            ColliderComponent_get(components, player->target)->enabled = false;
        } else {
            AmmoComponent_get(components, i)->size += ammo->size;
            clear_grid(components, grid, player->target);
            destroy_entity(components, player->target);
            player->target = -1;
        }
    } else {
        int i = find(-1, player->inventory, player->inventory_size);
        if (i != -1) {
            player->inventory[i] = player->target;
            CoordinateComponent* coord = CoordinateComponent_get(components, player->target);
            coord->parent = entity;
            coord->position = (sfVector2f) { 0.75, 0.0 };
            coord->angle = 0.0f;
            ColliderComponent_get(components, player->target)->enabled = false;
            change_layer(components, player->target, 6);
            ImageComponent_get(components, player->target)->outline = 0.0f;

            if (player->item != i) {
                ImageComponent_get(components, player->target)->alpha = 0.0;
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

        change_layer(components, i, 3);
        ImageComponent_get(components, i)->alpha = 1.0f;
        ColliderComponent_get(components, i)->enabled = true;
    }
}
