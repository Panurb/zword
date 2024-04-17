#define _USE_MATH_DEFINES

#include <math.h>

#include "component.h"
#include "util.h"
#include "collider.h"
#include "grid.h"
#include "enemy.h"
#include "sound.h"
#include "health.h"


void apply_force(int entity, Vector2f force) {
    PhysicsComponent* physics = PhysicsComponent_get(entity);

    Vector2f a = mult(1.0f / physics->mass, force);
    physics->acceleration = sum(physics->acceleration, a);
}


void update_physics(float time_step) {
    for (int i = 0; i < game_data->components->entities; i++) {
        PhysicsComponent* physics = PhysicsComponent_get(i);
        if (!physics) continue;

        ColliderComponent* col = ColliderComponent_get(i);

        physics->lifetime -= time_step;
        if (physics->lifetime <= 0.0f) {
            if (col) {
                clear_grid(i);
            }
            remove_children(i);
            destroy_entity(i);
            continue;
        } else if (physics->lifetime <= 1.0f) {
            ImageComponent* image = ImageComponent_get(i);
            if (image) {
                image->alpha = physics->lifetime;
            }
        }

        CoordinateComponent* coord = CoordinateComponent_get(i);
        if (coord->parent != -1) continue;

        if (physics->collision.entities->size > 0) {
            physics->velocity = physics->collision.velocity;

            Vector2f v_n = proj(physics->velocity, physics->collision.overlap);
            Vector2f v_t = diff(physics->velocity, v_n);
            physics->velocity = sum(mult(physics->bounce, v_n), mult(1.0f - physics->friction, v_t));

            blunt_damage(i, v_n);
        }

        Vector2f delta_pos = sum(physics->collision.overlap, mult(time_step, physics->velocity));
        float delta_angle = time_step * physics->angular_velocity;

        JointComponent* joint = JointComponent_get(i);
        if (joint && joint->parent != -1) {
            Vector2f r = diff(get_position(joint->parent), get_position(i));
            float d = norm(r);

            Vector2f f = zeros();
            if (d > joint->max_length) {
                f = mult(d - joint->max_length, normalized(r));
            } else if (d < joint->min_length) {
                f = mult(d - joint->min_length, normalized(r));
            }
            delta_pos = sum(delta_pos, mult(joint->strength, f));
            
            delta_angle = angle_diff(polar_angle(r), coord->angle);

            float angle = signed_angle(r, polar_to_cartesian(1.0f, get_angle(joint->parent)));
            if (fabsf(angle) > joint->max_angle) {
                r = polar_to_cartesian(d, get_angle(joint->parent) - sign(angle) * joint->max_angle);
                delta_pos = sum(delta_pos, diff(diff(get_position(joint->parent), r), coord->position));
            }
        }

        bool moved = col && col->enabled && (non_zero(delta_pos) || delta_angle != 0.0f);
        if (moved) {
            clear_grid(i);
        }
        coord->position = sum(coord->position, delta_pos);
        coord->angle = mod(coord->angle + delta_angle, 2.0f * M_PI);
        if (moved) {
            update_grid(i);
        }

        Vector2f v_hat = normalized(physics->velocity);
        Vector2f v_forward = proj(v_hat, polar_to_cartesian(1.0, coord->angle));
        Vector2f v_sideways = diff(v_hat, v_forward);
        Vector2f a = lin_comb(-physics->drag, v_forward, -physics->drag_sideways, v_sideways);
        physics->acceleration = sum(physics->acceleration, a);
        physics->velocity = sum(physics->velocity, mult(time_step, physics->acceleration));
        physics->acceleration = zeros();
        
        physics->speed = norm(physics->velocity);
        if (physics->speed < 0.1f) {
            physics->velocity = zeros();
            physics->speed = 0.0f;
        } else if (physics->speed > physics->max_speed) {
            physics->velocity = mult(physics->max_speed / physics->speed, physics->velocity);
            physics->speed = physics->max_speed;
        }

        physics->angular_acceleration -= sign(physics->angular_velocity) * physics->angular_drag;
        physics->angular_velocity += time_step * physics->angular_acceleration;
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
