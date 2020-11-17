#pragma once

#include <math.h>

#include "component.h"


void update(Component* component, float deltaTime) {
    for (int i = 0; i <= component->entities; i++) {
        if (!component->physics[i]) continue;

        component->physics[i]->acceleration.x -= copysignf(component->physics[i]->friction, component->physics[i]->velocity.x);
        component->physics[i]->acceleration.y -= copysignf(component->physics[i]->friction, component->physics[i]->velocity.y);

        component->physics[i]->velocity.x += component->physics[i]->acceleration.x * deltaTime;
        component->physics[i]->velocity.y += component->physics[i]->acceleration.y * deltaTime;

        component->coordinate[i]->position.x += component->physics[i]->velocity.x * deltaTime;
        component->coordinate[i]->position.y += component->physics[i]->velocity.y * deltaTime;

        //if (fabs(component->physics[i]->velocity.x) < 0.01) {
         //   component->physics[i]->velocity.x = 0.0;
        //}

        //if (fabs(component->physics[i]->velocity.y) < 0.01) {
         //   component->physics[i]->velocity.y = 0.0;
        //}

        component->physics[i]->acceleration.x = 0.0;
        component->physics[i]->acceleration.y = 0.0;
    }
}
