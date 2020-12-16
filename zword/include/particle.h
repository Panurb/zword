#pragma once

#include "camera.h"


void ParticleComponent_add_blood(ComponentData* components, int entity);

void ParticleComponent_add_sparks(ComponentData* components, int entity);

void ParticleComponent_add_dirt(ComponentData* components, int entity);

void update_particles(ComponentData* component, float delta_time);

void draw_particles(ComponentData* component, sfRenderWindow* window, Camera* camera, int entity);
