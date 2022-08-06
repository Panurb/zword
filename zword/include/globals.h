#pragma once

typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_PAUSE,
    STATE_QUIT
} GameState;


extern GameState game_state;
