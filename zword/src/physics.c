#include <math.h>

#include <SFML/System/Vector2.h>

#include "component.h"
#include "util.h"
#include "collider.h"


void update(Component* component, float delta_time, CollisionGrid collision_grid) {
    for (int i = 0; i < component->entities; i++) {
        PhysicsComponent* physics = component->physics[i];

        if (!physics) continue;

        CoordinateComponent* coord = component->coordinate[i];

        int x = floor(coord->position.x + 32);
        int y = floor(coord->position.y + 32);

        clear_grid(component, collision_grid, i);

        coord->position = sum(coord->position, physics->collision.overlap);

        if (fabs(physics->collision.velocity.x) != 0.0) {
            physics->velocity = physics->collision.velocity;

            sfVector2f v_n = proj(physics->velocity, physics->collision.overlap);
            sfVector2f v_t = diff(physics->velocity, v_n);
            physics->velocity = sum(mult(physics->bounce, v_n), mult(1.0 - physics->friction, v_t));
        }

        physics->collision.overlap = (sfVector2f) { 0.0, 0.0 };
        physics->collision.velocity = (sfVector2f) { 0.0, 0.0 };

        physics->acceleration = diff(physics->acceleration, mult(physics->drag, normalized(physics->velocity)));

        physics->velocity = sum(physics->velocity, mult(delta_time, physics->acceleration));

        coord->position = sum(coord->position, mult(delta_time, physics->velocity));

        update_grid(component, collision_grid, i);

        physics->acceleration = (sfVector2f) { 0.0, 0.0 };
    }
}
