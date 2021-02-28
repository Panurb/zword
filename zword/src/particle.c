#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "component.h"
#include "util.h"
#include "camera.h"


void ParticleComponent_add_blood(ComponentData* components, int entity) {
    sfColor color = get_color(0.78, 0.0, 0.0, 1.0);
    ParticleComponent_add(components, entity, 0.0, 2 * M_PI, 0.25, 0.0, 5.0, 10.0, color, color);
}


void ParticleComponent_add_sparks(ComponentData* components, int entity) {
    ParticleComponent_add(components, entity, 0.0, 2 * M_PI, 0.15, 0.0, 5.0, 5.0, sfWhite, sfWhite);
}


void ParticleComponent_add_dirt(ComponentData* components, int entity) {
    sfColor color = get_color(0.4, 0.25, 0.13, 1.0);
    ParticleComponent_add(components, entity, 0.0, 2 * M_PI, 0.25, 0.0, 2.5, 10.0, color, color);
}


void add_particle(ComponentData* components, int entity) {
    ParticleComponent* part = ParticleComponent_get(components, entity);

    part->position[part->iterator] = sum(get_position(components, entity), part->origin);
    float r = part->speed * randf(1.0 - part->speed_spread, 1.0 + part->speed_spread);
    float angle = randf(part->angle - 0.5 * part->spread, part->angle + 0.5 * part->spread);
    part->velocity[part->iterator] = polar_to_cartesian(r, get_angle(components, entity) + angle);
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


sfColor color_lerp(sfColor s, sfColor e, float t) {
    return sfColor_fromRGBA(lerp(s.r, e.r, t), lerp(s.g, e.g, t), lerp(s.b, e.b, t), lerp(s.a, e.a, t));
}


void draw_particles(ComponentData* components, sfRenderWindow* window, int camera, int entity) {
    ParticleComponent* part = components->particle[entity];
    if (!part) return;

    for (int i = part->particles - 1; i >= 0; i--) {
        if (part->time[i] == 0.0) continue;

        float t = 1.0 - part->time[i] / part->max_time;
        sfColor color = color_lerp(part->start_color, part->end_color, t);

        float r = lerp(part->start_size, part->end_size, t);
        float angle = polar_angle(part->velocity[i]);

        draw_ellipse(window, components, camera, part->shape, part->position[i], max(1.0, 0.1 * norm(part->velocity[i])) * r, r, angle, color);
    }

    for (int i = part->particles - 1; i >= 0; i--) {
        if (part->time[i] == 0.0) continue;

        float t = 1.0 - part->time[i] / part->max_time;
        sfColor color = color_lerp(part->start_color, part->end_color, t);
        color = color_lerp(color, sfWhite, 0.25);

        float r = 0.5 * lerp(part->start_size, part->end_size, t);
        float angle = polar_angle(part->velocity[i]);

        draw_ellipse(window, components, camera, part->shape, part->position[i], max(1.0, 0.1 * norm(part->velocity[i])) * r, r, angle, color);
    }
}
