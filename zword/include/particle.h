#pragma once

#include "camera.h"


void ParticleComponent_add_bullet(ComponentData* components, int entity, float size);

void ParticleComponent_add_blood(ComponentData* components, int entity);

void ParticleComponent_add_sparks(ComponentData* components, int entity);

void ParticleComponent_add_dirt(ComponentData* components, int entity);

void ParticleComponent_add_rock(ComponentData* components, int entity);

void ParticleComponent_add_splinter(ComponentData* components, int entity);

void ParticleComponent_add_fire(ComponentData* components, int entity, float size);

void ParticleComponent_add_energy(ComponentData* components, int entity);

void add_particles(ComponentData* components, int entity, int n);

void update_particles(ComponentData* components, int camera, float delta_time);

void draw_particles(ComponentData* components, sfRenderWindow* window, int camera, int entity);
