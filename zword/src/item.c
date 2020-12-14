#include "component.h"
#include "grid.h"
#include "util.h"


void create_flashlight(ComponentData* components, float x, float y) {
    int i = get_index(components);

    sfVector2f pos = { x, y };

    CoordinateComponent_add(components, i, pos, rand_angle());
    ColliderComponent_add_rectangle(components, i, 0.5, 0.25, ITEMS);
    PhysicsComponent_add(components, i, 0.5, 0.0, 0.5, 10.0, 2.5);
    ItemComponent_add(components, i, 0);
    LightComponent_add(components, i, 7.0, 1.0, 51, get_color(1.0, 1.0, 0.8, 1.0), 0.75, 10.0)->enabled = false;
    ImageComponent_add(components, i, "flashlight", 1.0, 1.0, 3);
}


void pick_up_item(ComponentData* components, int entity) {
    PlayerComponent* player = components->player[entity];

    if (player->target == -1) {
        return;
    }

    int i = find(-1, player->inventory, player->inventory_size);
    if (i != -1) {
        player->inventory[i] = player->target;
        CoordinateComponent* coord = CoordinateComponent_get(components, player->target);
        coord->parent = entity;
        coord->position = zeros();
        coord->angle = 0.0;
        ImageComponent_get(components, player->target)->alpha = 0.0;
        ColliderComponent_get(components, player->target)->enabled = false;
    }
}


void drop_item(ComponentData* components, int entity) {
    PlayerComponent* player = components->player[entity];

    int i = player->inventory[player->item];
    if (i != -1) {
        player->inventory[player->item] = -1;

        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        coord->parent = -1;

        sfVector2f r = polar_to_cartesian(1.0, get_angle(components, entity));
        coord->position = sum(get_position(components, entity), r);

        components->physics[i]->velocity = mult(7.0, r);
        components->physics[i]->angular_velocity = 3.0;

        if (components->light[i]) {
            components->light[i]->enabled = false;
        }

        ImageComponent_get(components, i)->alpha = 1.0;
        ColliderComponent_get(components, i)->enabled = true;
    }
}
