#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "particle.h"
#include "image.h"
#include "sound.h"
#include "navigation.h"


int create_car(ComponentData* components, sfVector2f pos, float angle) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, angle);
    ColliderComponent_add_circle(components, i, 1.5f, GROUP_VEHICLES);
    PhysicsComponent_add(components, i, 10.0f);

    int j = create_entity(components);
    CoordinateComponent_add(components, j, sum(pos, polar_to_cartesian(-3.0f, angle)), 0.0f);
    ColliderComponent_add_rectangle(components, j, 3.0f, 3.0f, GROUP_VEHICLES);
    PhysicsComponent_add(components, j, 10.0f);
    SoundComponent_add(components, j, "metal_hit");
    VehicleComponent_add(components, j, 100.0f);
    JointComponent_add(components, j, i, 3.0f, 3.0f, 1.0f);

    int k = create_entity(components);
    CoordinateComponent_add(components, k, (sfVector2f) {1.5f, 0.0f }, 0.0f);
    ImageComponent_add(components, k, "car", 6.0f, 3.0f, LAYER_VEHICLES);
    add_child(components, j, k);

    k = create_entity(components);
    CoordinateComponent_add(components, k, (sfVector2f) { 3.8f, 1.0f }, 0.0f);
    LightComponent_add(components, k, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;
    add_child(components, j, k);

    k = create_entity(components);
    CoordinateComponent_add(components, k, (sfVector2f) { 3.8f, -1.0f }, 0.0f);
    LightComponent_add(components, k, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;
    add_child(components, j, k);

    k = create_waypoint(components, (sfVector2f) { 5.0f, 2.5f });
    add_child(components, j, k);

    k = create_waypoint(components, (sfVector2f) { 5.0f, -2.5f });
    add_child(components, j, k);

    k = create_waypoint(components, (sfVector2f) { -2.0f, 2.5f });
    add_child(components, j, k);

    k = create_waypoint(components, (sfVector2f) { -2.0f, -2.5f });
    add_child(components, j, k);

    return i;
}


bool enter_vehicle(ComponentData* components, ColliderGrid* grid, int i) {
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
            clear_grid(components, grid, i);
            coord->position = vehicle->seats[closest];
            coord->angle = 0.0;
            coord->parent = j;
            ColliderComponent_get(components, i)->enabled = false;

            vehicle->riders[closest] = i;

            if (closest == 0) {
                player->state = PLAYER_DRIVE;
                loop_sound(components, j, "car", 0.5, 0.8);
            } else {
                player->state = PLAYER_PASSENGER;
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
    VehicleComponent* vehicle = VehicleComponent_get(components, player->vehicle);
    int i = JointComponent_get(components, player->vehicle)->parent;
    PhysicsComponent* phys = PhysicsComponent_get(components, i);

    float front_angle = get_angle(components, i);

    phys->acceleration = polar_to_cartesian(vehicle->acceleration * gas, front_angle);

    float max_speed = (0.5f + 0.5f * vehicle->on_road) * vehicle->max_speed;

    float vn = dot(phys->velocity, polar_to_cartesian(1.0f, front_angle));
    if (vn < 0.0f && phys->speed > 0.5f * vehicle->max_speed) {
        max_speed *= 0.5f;
    }

    if (phys->speed > max_speed) {
        phys->acceleration = zeros();
    }

    phys->velocity = polar_to_cartesian(vn, front_angle);

    float angle = get_angle(components, player->vehicle);
    float delta_angle = angle_diff(front_angle, angle);

    phys->angular_velocity = 4.0f * vehicle->turning * sign(-vehicle->turning * steering - delta_angle);

    vehicle->on_road = false;

    SoundComponent* scomp = SoundComponent_get(components, player->vehicle);
    for (int j = 0; j < scomp->size; j++) {
        SoundEvent* event = scomp->events[j];
        if (event && event->loop) {
            event->pitch = 0.8f + 0.4f * (phys->speed / vehicle->max_speed);
            break;
        }
    }
}
