#pragma once

#include "camera.h"


ParticleComponent* ParticleComponent_add_type(int entity, ParticleType type, float size);

void add_burst(Entity entity, Vector2f origin, float angle, int count, float max_time);

void add_particles(int entity, int n);

void update_particles(int camera, float delta_time);

void draw_particles(int camera);
