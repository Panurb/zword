#pragma once

#include "util.h"
#include "component.h"
#include "camera.h"
#include "settings.h"


extern char* ACTIONS[];
extern int ACTIONS_SIZE;
extern char* ACTION_BUTTONS_XBOX[];

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
    ACTION_AMMO
} PlayerAction;

char* key_to_string(sfKeyCode key);

char* keybind_to_string(Keybind keybind);

Keybind string_to_keybind(String string);

char* key_to_letter(sfKeyCode key);

char* action_to_keybind(char* action);

void replace_actions(String output, String input);

void input(int camera);
