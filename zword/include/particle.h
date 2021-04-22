#pragma once

#include "camera.h"


void ParticleComponent_add_blood(ComponentData* components, int entity);

void ParticleComponent_add_sparks(ComponentData* components, int entity);

void ParticleComponent_add_dirt(ComponentData* components, int entity);

void add_particles(ComponentData* components, int entity, int n);

void update_particles(ComponentData* components, float delta_time);

void draw_particles(ComponentData* components, sfRenderWindow* window, int camera, int entity);
