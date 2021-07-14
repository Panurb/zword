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
    SoundComponent_add(components, i, "metal_hit");

    int j = create_entity(components);
    CoordinateComponent_add(components, j, (sfVector2f) {-3.0f, 0.0f }, 0.0f);
    ColliderComponent_add_circle(components, j, 1.0f, GROUP_VEHICLES);
    PhysicsComponent_add(components, j, 10.0f);
    JointComponent_add(components, j, i, 3.0f, 3.0f, 1.0f);
    vehicle->rear = j;

    int k = create_entity(components);
    CoordinateComponent_add(components, k, (sfVector2f) {1.5f, 0.0f }, 0.0f);
    ImageComponent_add(components, k, "car", 6.0f, 3.0f, LAYER_VEHICLES);
    add_child(components, j, k);

    k = create_entity(components);
    CoordinateComponent_add(components, k, (sfVector2f) { 10.3f, 1.0f }, 0.0f);
    LightComponent_add(components, k, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;
    add_child(components, j, k);

    k = create_entity(components);
    CoordinateComponent_add(components, k, (sfVector2f) { 10.3f, -1.0f }, 0.0f);
    LightComponent_add(components, k, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;
    add_child(components, j, k);
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

    if (k == 0) {
        stop_loop(components, j);
    }
    add_sound(components, j, "car_door", 0.75, 1.0);
}


void drive_vehicle(ComponentData* components, int p, float gas, float steering) {
    PlayerComponent* player = PlayerComponent_get(components, p);
    int i = player->vehicle;

    VehicleComponent* vehicle = VehicleComponent_get(components, i);

    CoordinateComponent* coord = CoordinateComponent_get(components, i);
    PhysicsComponent* phys = PhysicsComponent_get(components, i);

    phys->acceleration = polar_to_cartesian(vehicle->acceleration * gas, coord->angle);

    float max_speed = (0.5f + 0.5f * vehicle->on_road) * vehicle->max_speed;

    float vn = dot(phys->velocity, polar_to_cartesian(1.0f, coord->angle));
    if (vn < 0.0f && phys->speed > 0.5f * vehicle->max_speed) {
        max_speed *= 0.5f;
    }

    if (phys->speed > max_speed) {
        phys->acceleration = zeros();
    }

    phys->velocity = polar_to_cartesian(vn, coord->angle);

    float angle = get_angle(components, vehicle->rear);
    float delta_angle = mod(coord->angle - angle, 2.0f * M_PI);
    if (delta_angle > M_PI) {
        delta_angle -= 2.0f * M_PI;
    }

    phys->angular_velocity = 4.0f * vehicle->turning * sign(-vehicle->turning * steering - delta_angle);


    vehicle->on_road = true;

    SoundComponent* scomp = SoundComponent_get(components, i);
    for (int j = 0; j < scomp->size; j++) {
        SoundEvent* event = scomp->events[j];
        if (event && event->loop) {
            event->pitch = 0.8f + 0.4f * (phys->speed / vehicle->max_speed);
            break;
        }
    }
}
