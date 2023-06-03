#pragma once

#include "camera.h"


void ParticleComponent_add_type(ComponentData* components, int entity, ParticleType type, float size);

void add_particles(ComponentData* components, int entity, int n);

void update_particles(ComponentData* components, int camera, float delta_time);

void draw_particles(ComponentData* components, sfRenderWindow* window, int camera, int entity);
