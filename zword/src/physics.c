#pragma once

#include <math.h>

#include "component.h"
#include "util.h"


void update(Component* component, float deltaTime) {
    for (int i = 0; i <= component->entities; i++) {
        PhysicsComponent* physics = component->physics[i];

        if (!physics) continue;

        component->coordinate[i]->position.x += physics->collision.overlap.x;
        component->coordinate[i]->position.y += physics->collision.overlap.y;

        physics->velocity.x += physics->collision.velocity.x;
        physics->velocity.y += physics->collision.velocity.y;

        if (fabs(physics->collision.velocity.x) > 1e-6) {
            physics->velocity.x *= 0.5;
            physics->velocity.y *= 0.5;

            physics->velocity.x *= physics->bounce;
            physics->velocity.y *= physics->bounce;
        }

        physics->collision.overlap.x = 0.0;
        physics->collision.overlap.y = 0.0;
        physics->collision.velocity.x = 0.0;
        physics->collision.velocity.y = 0.0;

        physics->acceleration.x -= copysignf(physics->friction, physics->velocity.x);
        physics->acceleration.y -= copysignf(physics->friction, physics->velocity.y);

        physics->velocity.x += physics->acceleration.x * deltaTime;
        physics->velocity.y += physics->acceleration.y * deltaTime;

        component->coordinate[i]->position.x += physics->velocity.x * deltaTime;
        component->coordinate[i]->position.y += physics->velocity.y * deltaTime;

        physics->acceleration.x = 0.0;
        physics->acceleration.y = 0.0;
    }
}
