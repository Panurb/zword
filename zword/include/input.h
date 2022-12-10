#pragma once

#include "util.h"
#include "component.h"
#include "camera.h"

typedef enum {
    ACTION_WALK_UP,
    ACTION_WALK_DOWN,
    ACTION_WALK_LEFT,
    ACTION_WALK_RIGHT,
    ACTION_
} Action;

typedef enum {
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_LB,
    BUTTON_RB,
    BUTTON_START,
    BUTTON_BACK,
    BUTTON_L,
    BUTTON_R,
    BUTTON_LT,
    BUTTON_RT
} ControllerButton;

void input(ComponentData* components, sfRenderWindow* window, int camera);
