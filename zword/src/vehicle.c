#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "particle.h"
#include "image.h"


void create_car(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, 0.0);
    ColliderComponent_add_rectangle(components, i, 5.0, 2.8, VEHICLES);
    PhysicsComponent_add(components, i, 10.0, 0.0, 0.0, 10.0, 20.0)->max_angular_speed = 2.5;
    VehicleComponent_add(components, i, 50.0);
    WaypointComponent_add(components, i);
    ImageComponent_add(components, i, "car", 6.0, 3.0, 4);
    ParticleComponent_add_sparks(components, i);

    i = create_entity(components);
    CoordinateComponent_add(components, i, (sfVector2f) { 2.8, 1.0 }, 0.0)->parent = i - 1;
    LightComponent_add(components, i, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;

    i = create_entity(components);
    CoordinateComponent_add(components, i, (sfVector2f) { 2.8, -1.0 }, 0.0)->parent = i - 2;
    LightComponent_add(components, i, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;

    /*
    i = create_entity(component);
    component->coordinate[i] = CoordinateComponent_create((sfVector2f) { 5.0, 2.0 }, 0.0);
    component->waypoint[i] = WaypointComponent_create();
    */
}


bool enter_vehicle(ComponentData* component, int i) {
    CoordinateComponent* coord = component->coordinate[i];

    for (int j = 0; j < component->entities; j++) {
        VehicleComponent* vehicle = component->vehicle[j];
        if (!vehicle)  continue;

        float min_d = 3.0;
        int closest = 0;
        for (int k = 0; k < 4; k++) {
            sfVector2f r = sum(get_position(component, j), rotate(vehicle->seats[k], get_angle(component, j)));
            float d = dist(get_position(component, i), r);

            if (d < min_d) {
                min_d = d;
                closest = k;
            }
        }

        if (min_d < 3.0) {
            int item = component->player[i]->inventory[component->player[i]->item];
            if (component->light[item]) {
                component->light[item]->enabled = false;
            }

            component->player[i]->vehicle = j;
            coord->position = vehicle->seats[closest];
            coord->angle = 0.0;
            coord->parent = j;
            component->collider[i]->enabled = false;

            vehicle->riders[closest] = i;

            return true;
        }
    }

    return false;
}


void exit_vehicle(ComponentData* component, int i) {
    int j = component->player[i]->vehicle;

    CoordinateComponent* coord = component->coordinate[i];
    coord->position = sum(coord->position, get_position(component, j));
    coord->parent = -1;

    component->player[i]->vehicle = -1;
    replace(i, -1, component->vehicle[j]->riders, 4);
    component->collider[i]->enabled = true;
}


void drive_vehicle(ComponentData* component, int i, sfVector2f v, float time_step) {
    int j = component->player[i]->vehicle;

    VehicleComponent* vehicle = component->vehicle[j];

    if (i != vehicle->riders[0]) return;

    if (vehicle->fuel == 0.0) return;

    PhysicsComponent* phys = component->physics[j];

    sfVector2f r = polar_to_cartesian(1.0, component->coordinate[j]->angle);
    sfVector2f at = mult(vehicle->acceleration * v.y, r);
    phys->acceleration = sum(phys->acceleration, at);   

    if (vehicle->on_road) {
        if (phys->speed > vehicle->max_speed) {
            phys->acceleration = zeros();
        }
    } else {
        if (phys->speed > 0.5 * vehicle->max_speed) {
            phys->acceleration = zeros();
        }
    }

    if (norm(phys->velocity) > 1.2) {
        phys->angular_acceleration -= sign(v.y + 0.1) * vehicle->turning * v.x;
    }

    sfVector2f v_new = rotate(phys->velocity, phys->angular_velocity * time_step);

    phys->acceleration = sum(phys->acceleration, mult(1.0 / time_step, diff(v_new, phys->velocity)));

    vehicle->fuel = fmax(0.0, vehicle->fuel - 0.1 * phys->speed * time_step);

    vehicle->on_road = false;
}
