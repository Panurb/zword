#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "component.h"
#include "util.h"
#include "camera.h"


sfColor color_lerp(sfColor s, sfColor e, float t) {
    return sfColor_fromRGBA(lerp(s.r, e.r, t), lerp(s.g, e.g, t), lerp(s.b, e.b, t), lerp(s.a, e.a, t));
}


void ParticleComponent_add_bullet(ComponentData* components, int entity, float size) {
    ParticleComponent_add(components, entity, 0.0f, 0.0f, size, size, 100.0f, 1, get_color(1.0f, 1.0f, 0.5f, 0.5f), sfWhite)->speed_spread = 0.0f;
}


void ParticleComponent_add_blood(ComponentData* components, int entity) {
    sfColor color = get_color(0.78, 0.0, 0.0, 1.0);
    sfColor inner_color = color_lerp(color, sfWhite, 0.25f);
    ParticleComponent_add(components, entity, 0.0, 2 * M_PI, 0.25, 0.0, 5.0, 10.0, color, inner_color);
}


void ParticleComponent_add_sparks(ComponentData* components, int entity) {
    ParticleComponent_add(components, entity, 0.0, 2 * M_PI, 0.15, 0.0, 5.0, 5.0, get_color(1.0f, 1.0f, 0.5f, 0.5f), sfWhite);
}


void ParticleComponent_add_dirt(ComponentData* components, int entity) {
    sfColor color = get_color(0.4, 0.25, 0.13, 0.5f);
    sfColor inner_color = color_lerp(color, sfWhite, 0.5f);
    ParticleComponent_add(components, entity, 0.0, 2 * M_PI, 0.25, 0.0, 2.5, 10.0, color, inner_color);
}


void ParticleComponent_add_rock(ComponentData* components, int entity) {
    sfColor color = get_color(0.4f, 0.4f, 0.4f, 1.0f);
    sfColor inner_color = color_lerp(color, sfWhite, 0.5f);
    ParticleComponent_add(components, entity, 0.0f, 2.0f * M_PI, 0.3f, 0.0f, 1.5f, 5.0f, color, inner_color);
}


void ParticleComponent_add_splinter(ComponentData* components, int entity) {
    sfColor color = get_color(0.5f, 0.4f, 0.3f, 1.0f);
    sfColor inner_color = color_lerp(color, sfWhite, 0.5f);
    ParticleComponent_add(components, entity, 0.0f, 2.0f * M_PI, 0.15f, 0.0f, 5.0f, 10.0f, color, inner_color);
}


void ParticleComponent_add_fire(ComponentData* components, int entity, float size) {
    sfColor orange = get_color(1.0, 0.6, 0.0, 1.0);
    LightComponent_add(components, entity, 30.0f * size, 2.0 * M_PI, orange, 2.0f * size, 10.0)->flicker = 0.2;
    float angle = 0.5f * M_PI - get_angle(components, entity);
    ParticleComponent* particle = ParticleComponent_add(components, entity, angle, 1.0, size, 0.25f * size, 1.0, 5.0, orange, orange);
    particle->loop = true;
    particle->enabled = true;
}


void add_particles(ComponentData* components, int entity, int n) {
    ParticleComponent* part = ParticleComponent_get(components, entity);

    for (int i = 0; i < n; i++) {
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
}


void update_particles(ComponentData* component, float delta_time) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->particle[i]) continue;

        ParticleComponent* part = component->particle[i];

        if (part->enabled) {
            if (part->loop) {
                while (part->timer <= 0.0) {
                    add_particles(component, i, 1);
                    part->timer += 1.0 / part->rate;
                }

                part->timer -= delta_time;
            } else {
                add_particles(component, i, part->rate);
                part->enabled = false;
            }
        }

        for (int j = 0; j < part->particles; j++) {
            part->position[j] = sum(part->position[j], mult(delta_time, part->velocity[j]));
            part->time[j] = fmax(0.0, part->time[j] - delta_time);
        }
    }
}


void draw_particles(ComponentData* components, sfRenderWindow* window, int camera, int entity) {
    ParticleComponent* part = components->particle[entity];
    if (!part) return;

    for (int i = part->particles - 1; i >= 0; i--) {
        if (part->time[i] == 0.0) continue;

        sfColor color = part->outer_color;

        float t = 1.0f - part->time[i] / part->max_time;
        float r = lerp(part->start_size, part->end_size, t);
        float angle = polar_angle(part->velocity[i]);

        draw_ellipse(window, components, camera, part->shape, part->position[i], fmaxf(1.0f, 0.1f * norm(part->velocity[i])) * r, r, angle, color);
    }

    for (int i = part->particles - 1; i >= 0; i--) {
        if (part->time[i] == 0.0) continue;

        sfColor color = part->inner_color;

        float t = 1.0f - part->time[i] / part->max_time;
        float r = 0.5f * lerp(part->start_size, part->end_size, t);
        float angle = polar_angle(part->velocity[i]);

        draw_ellipse(window, components, camera, part->shape, part->position[i], fmaxf(1.0f, 0.1f * norm(part->velocity[i])) * r, r, angle, color);
    }
}
