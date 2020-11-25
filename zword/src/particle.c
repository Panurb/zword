#include <stdio.h>

#include "component.h"
#include "util.h"
#include "camera.h"


void update_particles(Component* component, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->particle[i]) continue;

        CoordinateComponent* coord = component->coordinate[i];
        ParticleComponent* part = component->particle[i];

        if (part->rate > 0.0) {
            if (part->loop || part->particles < part->max_particles) {
                while (part->timer <= 0.0) {
                    part->position[part->iterator] = coord->position;
                    part->velocity[part->iterator] = polar_to_cartesian(5.0, float_rand(0.0, part->angle));
                    if (part->particles < part->max_particles) {
                        part->particles++;
                    }
                    part->iterator = (part->iterator + 1) % part->max_particles;

                    part->timer += 1.0 / part->rate;
                }

                part->timer -= delta_time;
            }
        }

        for (int j = 0; j < part->particles; j++) {
            part->position[j] = sum(part->position[j], mult(delta_time, part->velocity[j]));
        }
    }
}


void draw_particles(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->particle[i]) continue;

        ParticleComponent* part = component->particle[i];

        sfCircleShape_setRadius(part->shape, 0.1 * camera->zoom);

        for (int j = 0; j < part->particles; j++) {
            sfCircleShape_setPosition(part->shape, world_to_screen(part->position[j], camera));
            sfRenderWindow_drawCircleShape(window, part->shape, NULL);
        }
    }
}
