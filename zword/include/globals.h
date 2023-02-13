#pragma once

typedef enum {
    STATE_MENU,
    STATE_START,
    STATE_GAME,
    STATE_PAUSE,
    STATE_APPLY,
    STATE_LOAD,
    STATE_EDITOR,
    STATE_QUIT
} GameState;


extern GameState game_state;
