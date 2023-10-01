#pragma once

#include "component.h"

int animation_frames(Filename image);

void animate(ComponentData* components, float time_step);

void stop_animation(ComponentData* components, int entity);
