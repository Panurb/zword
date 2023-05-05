#pragma once

#include "util.h"
#include "component.h"
#include "camera.h"


extern char* ACTIONS[];

extern char* BUTTON_NAMES[];


typedef enum {
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_LB,
    BUTTON_RB,
    BUTTON_START,
    BUTTON_BACK,
    BUTTON_LT,
    BUTTON_RT,
    BUTTON_L,
    BUTTON_R
} ControllerButton;

typedef enum {
    ACTION_UP,
    ACTION_LEFT,
    ACTION_DOWN,
    ACTION_RIGHT,
    ACTION_ATTACK,
    ACTION_ENTER,
    ACTION_PICKUP,
    ACTION_RELOAD,
    ACTION_ATTACHMENT,
    ACTION_INVENTORY,
    ACTION_AMMO,
    ACTION_GRAB,
    ACTION_DROP
} PlayerAction;

char* key_to_string(sfKeyCode key);

char* key_to_letter(sfKeyCode key);

void input(ComponentData* components, sfRenderWindow* window, int camera);
