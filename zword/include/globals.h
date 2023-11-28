#pragma once

#include "util.h"

typedef enum {
    STATE_MENU,
    STATE_START,
    STATE_END,
    STATE_RESET,
    STATE_GAME,
    STATE_PAUSE,
    STATE_APPLY,
    STATE_CREATE,
    STATE_LOAD,
    STATE_EDITOR,
    STATE_QUIT,
    STATE_GAME_OVER
} GameState;


extern GameState game_state;
