#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "component.h"
#include "util.h"
#include "camera.h"


void add_particle(Component* component, int i) {
    ParticleComponent* part = component->particle[i];

    part->position[part->iterator] = get_position(component, i);
    float r = part->speed * float_rand(1.0 - part->speed_spread, 1.0 + part->speed_spread);
    float angle = float_rand(part->angle - 0.5 * part->spread, part->angle + 0.5 * part->spread);
    part->velocity[part->iterator] = polar_to_cartesian(r, get_angle(component, i) + angle);
    part->time[part->iterator] = 0.0;
    part->lifetime[part->iterator] = 1.0;
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
            part->time[j] = fmax(0.0, part->time[j] + delta_time);
            part->lifetime[j] = 1.0;
        }
    }
}


void draw_particles(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->particle[i]) continue;

        ParticleComponent* part = component->particle[i];

        for (int j = 0; j < part->particles; j++) {
            if (part->time[j] > part->lifetime[j]) continue;

            sfCircleShape_setPosition(part->shape, world_to_screen(part->position[j], camera));
            sfCircleShape_setRadius(part->shape, (1.0 - part->time[j] / part->lifetime[j]) * part->max_size * camera->zoom);
            sfVector2f scale = { 0.5 * norm(part->velocity[j]), 1.0 };
            sfCircleShape_setScale(part->shape, scale);
            float angle = atan2(part->velocity[j].y, part->velocity[j].x);
            sfCircleShape_setRotation(part->shape, -to_degrees(angle));
            sfRenderWindow_drawCircleShape(window, part->shape, NULL);
        }
    }
}