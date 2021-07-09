#define _USE_MATH_DEFINES

#include <math.h>

#include <SFML/System/Vector2.h>

#include "component.h"
#include "util.h"
#include "collider.h"
#include "grid.h"
#include "enemy.h"
#include "sound.h"
#include "health.h"


void apply_force(ComponentData* components, int entity, sfVector2f force) {
    PhysicsComponent* physics = PhysicsComponent_get(components, entity);

    sfVector2f a = mult(1.0f / physics->mass, force);
    physics->acceleration = sum(physics->acceleration, a);
}


void update(ComponentData* components, float delta_time, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        PhysicsComponent* physics = PhysicsComponent_get(components, i);
        if (!physics) continue;

        CoordinateComponent* coord = CoordinateComponent_get(components, i);
        if (coord->parent != -1) continue;

        JointComponent* joint = JointComponent_get(components, i);

        if (physics->collision.entities->size > 0) {
            physics->velocity = physics->collision.velocity;

            sfVector2f v_n = proj(physics->velocity, physics->collision.overlap);
            sfVector2f v_t = diff(physics->velocity, v_n);
            physics->velocity = sum(mult(physics->bounce, v_n), mult(1.0 - physics->friction, v_t));

            blunt_damage(components, grid, i, v_n);
        }

        sfVector2f delta_pos = sum(physics->collision.overlap, mult(delta_time, physics->velocity));

        sfVector2f v_hat = normalized(physics->velocity);
        sfVector2f v_forward = proj(v_hat, polar_to_cartesian(1.0, coord->angle));
        sfVector2f v_sideways = diff(v_hat, v_forward);
        sfVector2f a = lin_comb(-physics->drag, v_forward, -physics->drag_sideways, v_sideways);

        if (joint) {
            sfVector2f r = diff(get_position(components, joint->parent), get_position(components, i));
            float d = norm(r);

            if (d > joint->max_length) {
                sfVector2f f = mult(d - joint->max_length, normalized(r));
                if (joint->strength == INFINITY) {
                    delta_pos = sum(delta_pos, f);
                } else {
                    a = sum(a, mult(joint->strength / physics->mass, f));
                }
            } else if (d < joint->min_length) {
                sfVector2f f = mult(d - joint->min_length, normalized(r));
                if (joint->strength == INFINITY) {
                    delta_pos = sum(delta_pos, f);
                } else {
                    a = sum(a, mult(joint->strength / physics->mass, f));
                }
            }

            coord->angle = polar_angle(r);

            float angle = signed_angle(r, polar_to_cartesian(1.0f, get_angle(components, joint->parent)));
            if (fabsf(angle) > joint->max_angle) {
                r = polar_to_cartesian(d, get_angle(components, joint->parent) - sign(angle) * joint->max_angle);
                delta_pos = sum(delta_pos, diff(diff(get_position(components, joint->parent), r), coord->position));
            }
        }

        if (ColliderComponent_get(components, i) && non_zero(delta_pos)) {
            clear_grid(components, grid, i);
            coord->position = sum(coord->position, delta_pos);
            update_grid(components, grid, i);
        } else {
            coord->position = sum(coord->position, delta_pos);
        }

        physics->acceleration = sum(physics->acceleration, a);
        physics->velocity = sum(physics->velocity, mult(delta_time, physics->acceleration));
        physics->acceleration = zeros();
        
        physics->speed = norm(physics->velocity);
        if (physics->speed < 0.1f) {
            physics->velocity = zeros();
            physics->speed = 0.0f;
        } else if (physics->speed > physics->max_speed) {
            physics->velocity = mult(physics->max_speed / physics->speed, physics->velocity);
            physics->speed = physics->max_speed;
        }

        if (!joint) {
            coord->angle = mod(coord->angle + delta_time * physics->angular_velocity, 2.0f * M_PI);
        }

        physics->angular_acceleration -= sign(physics->angular_velocity) * physics->angular_drag;
        physics->angular_velocity += delta_time * physics->angular_acceleration;
        physics->angular_acceleration = 0.0f;

        float angular_speed = fabs(physics->angular_velocity);
        if (angular_speed < 0.1f) {
            physics->angular_velocity = 0.0f;
        } else if (angular_speed > physics->max_angular_speed) {
            physics->angular_velocity = physics->max_angular_speed * sign(physics->angular_velocity);
        }

        // move up?
        if (physics->collision.entities->size > 0) {
            physics->angular_velocity = 0.0f;
        }
        
        List_clear(physics->collision.entities);
        physics->collision.overlap = zeros();
        physics->collision.velocity = zeros();
    }
}
