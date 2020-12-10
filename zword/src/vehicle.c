#include "component.h"


void create_car(ComponentData* component, float x, float y) {
    int i = get_index(component);
    sfVector2f pos = { x, y };
    component->coordinate[i] = CoordinateComponent_create(pos, 0.0);
    component->collider[i] = ColliderComponent_create_rectangle(6.0, 3.0);
    component->physics[i] = PhysicsComponent_create(10.0, 0.0, 0.5, 10.0, 20.0);
    component->physics[i]->max_angular_speed = 2.5;
    component->vehicle[i] = VehicleComponent_create();
    component->waypoint[i] = WaypointComponent_create();
    component->image[i] = ImageComponent_create("car", 6.0, 3.0, 3);
    //component->image[i]->shine = 1.0;

    i = get_index(component);
    component->coordinate[i] = CoordinateComponent_create((sfVector2f) { 3.1, 1.0 }, 0.0);
    component->coordinate[i]->parent = i - 1;
    component->light[i] = LightComponent_create(10.0, 1.0, 51, sfWhite, 0.4, 1.0);
    component->light[i]->enabled = false;

    i = get_index(component);
    component->coordinate[i] = CoordinateComponent_create((sfVector2f) { 3.1, -1.0 }, 0.0);
    component->coordinate[i]->parent = i - 2;
    component->light[i] = LightComponent_create(10.0, 1.0, 51, sfWhite, 0.4, 1.0);
    component->light[i]->enabled = false;

    /*
    i = get_index(component);
    component->coordinate[i] = CoordinateComponent_create((sfVector2f) { 5.0, 2.0 }, 0.0);
    component->waypoint[i] = WaypointComponent_create();
    */
}
