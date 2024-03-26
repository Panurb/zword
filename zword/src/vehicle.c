#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "particle.h"
#include "image.h"
#include "sound.h"
#include "navigation.h"
#include "game.h"


int create_car(Vector2f pos, float angle) {
    int i = create_entity();
    CoordinateComponent_add(i, pos, angle);
    ColliderComponent_add_circle(i, 1.5f, GROUP_VEHICLES);
    PhysicsComponent_add(i, 10.0f);

    int j = create_entity();
    CoordinateComponent_add(j, sum(pos, polar_to_cartesian(-3.0f, angle)), 0.0f);
    ColliderComponent_add_rectangle(j, 3.0f, 3.0f, GROUP_VEHICLES);
    PhysicsComponent_add(j, 10.0f);
    SoundComponent_add(j, "metal_hit");
    VehicleComponent_add(j, 100.0f);
    JointComponent_add(j, i, 3.0f, 3.0f, 1.0f);

    int k = create_entity();
    CoordinateComponent_add(k, (Vector2f) {1.5f, 0.0f }, 0.0f);
    ImageComponent_add(k, "car", 6.0f, 3.0f, LAYER_VEHICLES);
    add_child(j, k);

    k = create_entity();
    CoordinateComponent_add(k, (Vector2f) { 3.8f, 1.0f }, 0.0f);
    LightComponent_add(k, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;
    add_child(j, k);

    k = create_entity();
    CoordinateComponent_add(k, (Vector2f) { 3.8f, -1.0f }, 0.0f);
    LightComponent_add(k, 10.0, 1.0, sfWhite, 0.4, 1.0)->enabled = false;
    add_child(j, k);

    k = create_waypoint((Vector2f) { 5.0f, 2.5f });
    add_child(j, k);

    k = create_waypoint((Vector2f) { 5.0f, -2.5f });
    add_child(j, k);

    k = create_waypoint((Vector2f) { -2.0f, 2.5f });
    add_child(j, k);

    k = create_waypoint((Vector2f) { -2.0f, -2.5f });
    add_child(j, k);

    return i;
}


bool enter_vehicle(int i) {
    CoordinateComponent* coord = CoordinateComponent_get(i);

    for (int j = 0; j < game_data->components->entities; j++) {
        VehicleComponent* vehicle = VehicleComponent_get(j);
        if (!vehicle)  continue;

        float min_d = 3.0;
        int closest = 0;
        for (int k = 0; k < 4; k++) {
            Vector2f r = sum(get_position(j), rotate(vehicle->seats[k], get_angle(j)));
            float d = dist(get_position(i), r);

            if (d < min_d) {
                min_d = d;
                closest = k;
            }
        }

        if (min_d < 3.0) {
            PlayerComponent* player = PlayerComponent_get(i);
            int item = player->inventory[player->item];
            LightComponent* light = LightComponent_get(item);
            if (light) {
                light->enabled = false;
            }

            player->vehicle = j;
            clear_grid(i);
            coord->position = vehicle->seats[closest];
            coord->angle = 0.0;
            coord->parent = j;
            ColliderComponent_get(i)->enabled = false;

            vehicle->riders[closest] = i;

            if (closest == 0) {
                player->state = PLAYER_DRIVE;
                add_sound(j, "car", 0.5, 0.8);
            } else {
                player->state = PLAYER_PASSENGER;
            }
            add_sound(j, "car_door", 0.75, 1.0);

            PhysicsComponent_get(i)->velocity = zeros();
            return true;
        }
    }

    return false;
}


void exit_vehicle(int i) {
    PlayerComponent* player = PlayerComponent_get(i);
    int j = player->vehicle;
    VehicleComponent* vehicle = VehicleComponent_get(j);

    int k = find(i, vehicle->riders, vehicle->size);

    CoordinateComponent* coord = CoordinateComponent_get(i);
    Vector2f r = vehicle->seats[k];
    r.y *= 2.0;
    r = rotate(r, get_angle(j));
    coord->position = sum(get_position(j), r);
    coord->parent = -1;

    player->vehicle = -1;
    vehicle->riders[k] = -1;
    ColliderComponent_get(i)->enabled = true;

    if (k == 0) {
        stop_loop(j);
    }
    add_sound(j, "car_door", 0.75, 1.0);
}


void drive_vehicle(int p, float gas, float steering) {
    PlayerComponent* player = PlayerComponent_get(p);
    VehicleComponent* vehicle = VehicleComponent_get(player->vehicle);
    int i = JointComponent_get(player->vehicle)->parent;
    PhysicsComponent* phys = PhysicsComponent_get(i);

    float front_angle = get_angle(i);

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

    float angle = get_angle(player->vehicle);
    float delta_angle = angle_diff(front_angle, angle);

    phys->angular_velocity = 4.0f * vehicle->turning * sign(-vehicle->turning * steering - delta_angle);

    vehicle->on_road = false;

    SoundComponent* scomp = SoundComponent_get(player->vehicle);
    for (int j = 0; j < scomp->size; j++) {
        SoundEvent* event = scomp->events[j];
        if (event && event->loop) {
            event->pitch = 0.8f + 0.4f * (phys->speed / vehicle->max_speed);
            break;
        }
    }
}
