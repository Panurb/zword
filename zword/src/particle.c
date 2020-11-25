#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "component.h"
#include "util.h"
#include "camera.h"


void add_particle(Component* component, int i) {
    CoordinateComponent* coord = component->coordinate[i];
    ParticleComponent* part = component->particle[i];

    part->position[part->iterator] = coord->position;
    part->velocity[part->iterator] = polar_to_cartesian(float_rand(2.0, 4.0), coord->angle + float_rand(-0.5 * part->angle, 0.5 * part->angle));
    part->size[part->iterator] = part->max_size;
    if (part->particles < part->max_particles) {
        part->particles++;
    }
    part->iterator = (part->iterator + 1) % part->max_particles;
}


void update_particles(Component* component, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->particle[i]) continue;

        CoordinateComponent* coord = component->coordinate[i];
        ParticleComponent* part = component->particle[i];

        if (part->enabled) {
            if (part->loop) {
                while (part->timer <= 0.0) {
                    add_particle(component, i);
                    part->timer += 1.0 / part->rate;
                }

                part->timer -= delta_time;
            } else {
                for (int j = 0; j < part->rate; j++) {
                    add_particle(component, i);
                }
                part->enabled = false;
            }
        }

        for (int j = 0; j < part->particles; j++) {
            part->position[j] = sum(part->position[j], mult(delta_time, part->velocity[j]));
            part->size[j] = fmax(0.0, part->size[j] - 0.1 * delta_time);
        }
    }
}


void draw_particles(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->particle[i]) continue;

        ParticleComponent* part = component->particle[i];

        for (int j = 0; j < part->particles; j++) {
            sfCircleShape_setPosition(part->shape, world_to_screen(part->position[j], camera));
            sfCircleShape_setRadius(part->shape, part->size[j] * camera->zoom);
            sfRenderWindow_drawCircleShape(window, part->shape, NULL);
        }
    }
}
