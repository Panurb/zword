#include "component.h"


void update_vehicles(Component* component, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        VehicleComponent* vehicle = component->vehicle[i];

        if (!vehicle) continue;
    }
}


void create_car(Component* component, float x, float y) {
    int i = component->entities;
    component->entities++;

    sfVector2f pos = { x, y };
    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->rectangle_collider[i] = RectangleColliderComponent_create(6.0, 3.0);
    component->physics[i] = PhysicsComponent_create(10.0, 0.0, 0.5, 10.0, 20.0);
    component->physics[i]->max_angular_speed = 2.5;
    component->vehicle[i] = VehicleComponent_create();

    i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create((sfVector2f) { 3.1, 1.0 }, 0.0);
    component->coordinate[i]->parent = i - 1;
    component->light[i] = LightComponent_create(10.0, 1.0, 51, sfWhite, 0.4, 1.0);
    component->light[i]->enabled = false;

    i = component->entities;
    component->entities++;

    component->coordinate[i] = CoordinateComponent_create((sfVector2f) { 3.1, -1.0 }, 0.0);
    component->coordinate[i]->parent = i - 2;
    component->light[i] = LightComponent_create(10.0, 1.0, 51, sfWhite, 0.4, 1.0);
    component->light[i]->enabled = false;
}
