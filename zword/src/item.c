#include "component.h"


void pick_up_item(ComponentData* components, int entity) {
    PlayerComponent* player = components->player[entity];

    for (int i = 0; i < components->entities; i++) {
        if (components->item[i] && components->coordinate[i]->parent == -1) {
            float d = dist(components->coordinate[entity]->position, components->coordinate[i]->position);
            if (d < 1.0) {
                int j = find(-1, player->inventory, player->inventory_size);
                if (j != -1) {
                    player->inventory[j] = i;
                    CoordinateComponent* coord = CoordinateComponent_get(components, i);
                    coord->parent = entity;
                    coord->position = zeros();
                    coord->angle = 0.0;
                    ImageComponent_get(components, i)->visible = false;
                }
                break;
            }
        }
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

        ImageComponent_get(components, i)->visible = true;
    }
}
