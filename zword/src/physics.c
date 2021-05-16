#define _USE_MATH_DEFINES

#include <math.h>

#include <SFML/System/Vector2.h>

#include "component.h"
#include "util.h"
#include "collider.h"
#include "grid.h"
#include "enemy.h"
#include "sound.h"


void apply_force(ComponentData* components, int entity, sfVector2f force) {
    PhysicsComponent* physics = PhysicsComponent_get(components, entity);

    sfVector2f a = mult(1.0f / physics->mass, force);
    physics->acceleration = sum(physics->acceleration, a);
}


void update(ComponentData* components, float delta_time, ColliderGrid* grid) {
    for (int i = 0; i < components->entities; i++) {
        PhysicsComponent* physics = components->physics[i];
        if (!physics) continue;

        CoordinateComponent* coord = components->coordinate[i];
        if (coord->parent != -1) continue;

        if (physics->collision.collided) {
            physics->velocity = physics->collision.velocity;

            sfVector2f v_n = proj(physics->velocity, physics->collision.overlap);
            sfVector2f v_t = diff(physics->velocity, v_n);
            physics->velocity = sum(mult(physics->bounce, v_n), mult(1.0 - physics->friction, v_t));

            HealthComponent* health = components->health[i];
            if (health) {
                float v = norm(v_n);
                if (v > 10.0f) {
                    if (health->health > 0) {
                        SoundComponent* sound = SoundComponent_get(components, i);
                        if (sound) {
                            add_sound(components, i, sound->hit_sound, 1.0, 0.8);
                        }
                    }
                    damage(components, i, get_position(components, i), v_n, 100);
                }
            }

            // SoundComponent* sound = SoundComponent_get(components, i);
            // if (sound) {
            //     if (physics->speed > 5.0f) {
            //         add_sound(components, i, sound->hit_sound, 0.5f, 1.0f);
            //     }
            // }
        }

        sfVector2f delta_pos = sum(physics->collision.overlap, mult(delta_time, physics->velocity));

        if (ColliderComponent_get(components, i) && non_zero(delta_pos)) {
            clear_grid(components, grid, i);
            for (ListNode* node = coord->children->head; node; node = node->next) {
                clear_grid(components, grid, node->value);
            }
            coord->position = sum(coord->position, delta_pos);
            update_grid(components, grid, i);
            for (ListNode* node = coord->children->head; node; node = node->next) {
                update_grid(components, grid, node->value);
            }
        } else {
            coord->position = sum(coord->position, delta_pos);
        }

        sfVector2f v_hat = normalized(physics->velocity);
        sfVector2f v_forward = proj(v_hat, polar_to_cartesian(1.0, coord->angle));
        sfVector2f v_sideways = diff(v_hat, v_forward);
        sfVector2f a = sum(mult(physics->drag, v_forward), mult(physics->drag_sideways, v_sideways));

        physics->acceleration = diff(physics->acceleration, a);

        physics->velocity = sum(physics->velocity, mult(delta_time, physics->acceleration));

        physics->acceleration = zeros();
        
        physics->speed = norm(physics->velocity);
        if (physics->speed < 0.1f) {
            physics->velocity = zeros();
            physics->speed = 0.0;
        } else if (physics->speed > physics->max_speed) {
            physics->velocity = mult(physics->max_speed / physics->speed, physics->velocity);
            physics->speed = physics->max_speed;
        }

        coord->angle += delta_time * physics->angular_velocity;

        physics->angular_acceleration -= sign(physics->angular_velocity) * physics->angular_drag;

        physics->angular_velocity += delta_time * physics->angular_acceleration;

        float angular_speed = fabs(physics->angular_velocity);

        if (angular_speed < 0.1) {
            physics->angular_velocity = 0.0;
        } else if (angular_speed > physics->max_angular_speed) {
            physics->angular_velocity = physics->max_angular_speed * sign(physics->angular_velocity);
        }

        if (physics->collision.collided) {
            physics->angular_velocity = 0.0;
        }
        
        physics->angular_acceleration = 0.0;

        physics->collision.collided = false;
        physics->collision.overlap = zeros();
        physics->collision.velocity = zeros();
    }
}
