#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "particle.h"
#include "image.h"
#include "sound.h"
#include "navigation.h"


void create_car(ComponentData* components, sfVector2f pos) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, 0.0f);
    ColliderComponent_add_circle(components, i, 1.0f, GROUP_VEHICLES);
    PhysicsComponent_add(components, i, 10.0f);
    VehicleComponent* vehicle = VehicleComponent_add(components, i, 100.0f);
    SoundComponent_add(components, i, "metal");

    int j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {-3.0f, 0.0f }, 0.0f);
    ColliderComponent_add_circle(components, j, 1.0f, GROUP_VEHICLES);
    PhysicsComponent_add(components, j, 10.0f);
    JointComponent_add(components, j, i, 3.0f, 3.0f, INFINITY);
    vehicle->rear = j;

    j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {-1.5f, 0.0f }, 0.0f);
    ImageComponent_add(components, j, "car", 6.0f, 3.0f, LAYER_VEHICLES);
    add_child(components, i, j);

    j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) { 1.3f, 1.0 }, 0.0);
    LightComponent_add(components, j, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;
    add_child(components, i, j);

    j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) { 1.3f, -1.0 }, 0.0)->parent = i;
    LightComponent_add(components, j, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;
}


bool enter_vehicle(ComponentData* components, int i) {
    CoordinateComponent* coord = CoordinateComponent_get(components, i);

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
            LightComponent* light = LightComponent_get(components, item);
            if (light) {
                light->enabled = false;
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

            PhysicsComponent_get(components, i)->velocity = zeros();
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
    r.y *= 2.0;
    r = rotate(r, get_angle(components, j));
    coord->position = sum(get_position(components, j), r);
    coord->parent = -1;

    player->vehicle = -1;
    vehicle->riders[k] = -1;
    components->collider[i]->enabled = true;

    stop_loop(components, j);
    add_sound(components, j, "car_door", 0.75, 1.0);
}


void drive_vehicle(ComponentData* components, int p, float gas, float steering, float time_step) {
    PlayerComponent* player = PlayerComponent_get(components, p);
    int i = player->vehicle;

    VehicleComponent* vehicle = VehicleComponent_get(components, i);

    CoordinateComponent* coord = CoordinateComponent_get(components, i);
    PhysicsComponent* phys = PhysicsComponent_get(components, i);

    coord->angle = polar_angle(diff(get_position(components, i), get_position(components, vehicle->rear)));
    sfVector2f an = polar_to_cartesian(vehicle->acceleration * gas, coord->angle);

    float speed = dot(phys->velocity, polar_to_cartesian(1.0f, coord->angle));
    float angle = coord->angle - sign(vehicle->turning) * 0.5f * M_PI;
    sfVector2f at = polar_to_cartesian(5.0f * speed * steering * vehicle->turning, angle);

    phys->acceleration = sum(an, at);

    if (vehicle->on_road) {
        if (phys->speed > vehicle->max_speed) {
            phys->acceleration = zeros();
        }
    } else {
        if (phys->speed > 0.5f * vehicle->max_speed) {
            phys->acceleration = zeros();
        }
    }

    // phys->velocity = polar_to_cartesian(phys->speed, angle);

    vehicle->on_road = true;

    SoundComponent* scomp = SoundComponent_get(components, i);
    for (int j = 0; j < scomp->size; j++) {
        SoundEvent* event = scomp->events[j];
        if (event && event->loop) {
            event->pitch = 0.8 + 0.4 * (phys->speed / vehicle->max_speed);
            break;
        }
    }
}
