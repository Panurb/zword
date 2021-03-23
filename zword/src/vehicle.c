#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "particle.h"
#include "image.h"
#include "sound.h"


void create_car(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);

    CoordinateComponent_add(components, i, pos, 0.5 * M_PI);
    ColliderComponent_add_rectangle(components, i, 5.0, 2.8, VEHICLES);
    PhysicsComponent_add(components, i, 10.0, 0.0, 0.0, 10.0, 20.0)->max_angular_speed = 2.5;
    VehicleComponent_add(components, i, 100.0);
    // WaypointComponent_add(components, i);
    ImageComponent_add(components, i, "car", 6.0, 3.0, 4);
    ParticleComponent_add_sparks(components, i);
    SoundComponent_add(components, i, "metal");

    i = create_entity(components);
    CoordinateComponent_add(components, i, (sfVector2f) { 2.8, 1.0 }, 0.0)->parent = i - 1;
    LightComponent_add(components, i, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;

    i = create_entity(components);
    CoordinateComponent_add(components, i, (sfVector2f) { 2.8, -1.0 }, 0.0)->parent = i - 2;
    LightComponent_add(components, i, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;
}


bool enter_vehicle(ComponentData* components, int i) {
    CoordinateComponent* coord = components->coordinate[i];

    for (int j = 0; j < components->entities; j++) {
        VehicleComponent* vehicle = components->vehicle[j];
        if (!vehicle)  continue;

        float min_d = 3.0;
        int closest = 0;
        for (int k = 0; k < 4; k++) {
            sfVector2f r = sum(get_position(components, j), rotate(vehicle->seats[k], get_angle(components, j)));
            float d = dist(get_position(components, i), r);

            if (d < min_d) {
                min_d = d;
                closest = k;
            }
        }

        if (min_d < 3.0) {
            PlayerComponent* player = PlayerComponent_get(components, i);
            int item = player->inventory[player->item];
            if (components->light[item]) {
                components->light[item]->enabled = false;
            }

            player->vehicle = j;
            coord->position = vehicle->seats[closest];
            coord->angle = 0.0;
            coord->parent = j;
            components->collider[i]->enabled = false;

            vehicle->riders[closest] = i;

            if (closest == 0) {
                loop_sound(components, j, "car", 0.5, 0.8);
            }
            add_sound(components, j, "car_door", 0.75, 1.0);

            return true;
        }
    }

    return false;
}


void exit_vehicle(ComponentData* components, int i) {
    PlayerComponent* player = PlayerComponent_get(components, i);
    int j = player->vehicle;
    VehicleComponent* vehicle = VehicleComponent_get(components, j);

    int k = find(i, vehicle->riders, vehicle->size);

    CoordinateComponent* coord = components->coordinate[i];
    sfVector2f r = vehicle->seats[k];
    r.x *= 2.0;
    r = rotate(r, get_angle(components, j));
    coord->position = sum(get_position(components, j), r);
    coord->parent = -1;

    player->vehicle = -1;
    vehicle->riders[k] = -1;
    components->collider[i]->enabled = true;

    stop_loop(components, j);
    add_sound(components, j, "car_door", 0.75, 1.0);
}


void drive_vehicle(ComponentData* components, int p, sfVector2f v, float time_step) {
    PlayerComponent* player = PlayerComponent_get(components, p);
    int i = player->vehicle;

    VehicleComponent* vehicle = components->vehicle[i];

    if (p != vehicle->riders[0]) return;

    if (vehicle->fuel == 0.0) {
        stop_loop(components, i);
        return;
    }

    PhysicsComponent* phys = components->physics[i];

    sfVector2f r = polar_to_cartesian(1.0, components->coordinate[i]->angle);
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

    SoundComponent* scomp = SoundComponent_get(components, i);
    for (int j = 0; j < scomp->size; j++) {
        SoundEvent* event = scomp->events[j];
        if (event && event->loop) {
            event->pitch = 0.8 + 0.4 * (phys->speed / vehicle->max_speed);
            break;
        }
    }
}
