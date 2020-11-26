#include <math.h>

#include <SFML/System/Vector2.h>

#include "component.h"
#include "util.h"
#include "collider.h"
#include "grid.h"


void update(Component* component, float delta_time, ColliderGrid* collision_grid) {
    for (int i = 0; i < component->entities; i++) {
        PhysicsComponent* physics = component->physics[i];

        if (!physics) continue;

        CoordinateComponent* coord = component->coordinate[i];

        if (physics->collision.velocity.x != 0.0 || physics->collision.velocity.y != 0.0) {
            physics->velocity = physics->collision.velocity;

            sfVector2f v_n = proj(physics->velocity, physics->collision.overlap);
            sfVector2f v_t = diff(physics->velocity, v_n);
            physics->velocity = sum(mult(physics->bounce, v_n), mult(1.0 - physics->friction, v_t));
        }

        physics->acceleration = diff(physics->acceleration, mult(physics->drag, normalized(physics->velocity)));

        physics->velocity = sum(physics->velocity, mult(delta_time, physics->acceleration));

        float v = norm(physics->velocity);
        if (v < 1e-3) {
            physics->velocity = (sfVector2f) { 0.0, 0.0 };
        } else if (v > physics->max_speed) {
            physics->velocity = mult(physics->max_speed / v, physics->velocity);
        }

        sfVector2f delta_pos = sum(physics->collision.overlap, mult(delta_time, physics->velocity));

        if (delta_pos.x != 0.0 || delta_pos.y != 0.0) {
            clear_grid(component, collision_grid, i);
        }

        coord->position = sum(coord->position, delta_pos);

        if (delta_pos.x != 0.0 || delta_pos.y != 0.0) {
            update_grid(component, collision_grid, i);
        }

        physics->collision.overlap = (sfVector2f) { 0.0, 0.0 };
        physics->collision.velocity = (sfVector2f) { 0.0, 0.0 };
        physics->acceleration = (sfVector2f) { 0.0, 0.0 };

        if (physics->angular_velocity != 0.0) {
            physics->angular_acceleration -= 2.0 * sign(physics->angular_velocity) * physics->drag;
        }

        physics->angular_velocity += delta_time * physics->angular_acceleration;
        if (fabs(physics->angular_velocity) > physics->max_angular_speed) {
            physics->angular_velocity = physics->max_angular_speed * sign(physics->angular_velocity);
        }
        
        coord->angle += delta_time * physics->angular_velocity;
        physics->angular_acceleration = 0.0;
    }
}
