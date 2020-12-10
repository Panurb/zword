#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "component.h"
#include "util.h"
#include "camera.h"


void add_particle(ComponentData* component, int i) {
    ParticleComponent* part = component->particle[i];

    part->position[part->iterator] = get_position(component, i);
    float r = part->speed * float_rand(1.0 - part->speed_spread, 1.0 + part->speed_spread);
    float angle = float_rand(part->angle - 0.5 * part->spread, part->angle + 0.5 * part->spread);
    part->velocity[part->iterator] = polar_to_cartesian(r, get_angle(component, i) + angle);
    part->time[part->iterator] = part->max_time;
    if (part->particles < part->max_particles) {
        part->particles++;
    }
    part->iterator = (part->iterator + 1) % part->max_particles;
}


void update_particles(ComponentData* component, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->particle[i]) continue;

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
            part->time[j] = fmax(0.0, part->time[j] - delta_time);
        }
    }
}


void draw_particles(ComponentData* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->particle[i]) continue;

        ParticleComponent* part = component->particle[i];

        sfCircleShape_setFillColor(part->shape, part->color);
        for (int j = 0; j < part->particles; j++) {
            if (part->time[j] == 0.0) continue;

            float r = (part->time[j] * part->max_size + (1 - part->time[j]) * part->min_size) * camera->zoom;
            sfCircleShape_setRadius(part->shape, r);
            sfCircleShape_setOrigin(part->shape, (sfVector2f) { r, r });
            sfVector2f pos = world_to_screen(part->position[j], camera);
            sfCircleShape_setPosition(part->shape, pos);
            sfVector2f scale = { max(1.0, 0.1 * norm(part->velocity[j])), 1.0 };
            sfCircleShape_setScale(part->shape, scale);
            float angle = atan2(part->velocity[j].y, part->velocity[j].x);
            sfCircleShape_setRotation(part->shape, -to_degrees(angle));
            sfRenderWindow_drawCircleShape(window, part->shape, NULL);
        }

        sfCircleShape_setFillColor(part->shape, part->inner_color);
        for (int j = 0; j < part->particles; j++) {
            if (part->time[j] == 0.0) continue;

            float r = 0.5 * (part->time[j] * part->max_size + (1 - part->time[j]) * part->min_size) * camera->zoom;
            sfCircleShape_setRadius(part->shape, r);
            sfCircleShape_setOrigin(part->shape, (sfVector2f) { r, r });
            sfVector2f pos = world_to_screen(part->position[j], camera);
            sfCircleShape_setPosition(part->shape, pos);
            sfVector2f scale = { max(1.0, 0.1 * norm(part->velocity[j])), 1.0 };
            sfCircleShape_setScale(part->shape, scale);
            float angle = atan2(part->velocity[j].y, part->velocity[j].x);
            sfCircleShape_setRotation(part->shape, -to_degrees(angle));
            sfRenderWindow_drawCircleShape(window, part->shape, NULL);
        }
    }
}
