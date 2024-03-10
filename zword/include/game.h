#pragma once

#include "component.h"
#include "collider.h"
#include "image.h"
#include "sound.h"


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


typedef enum {
    MODE_SURVIVAL,
    MODE_CAMPAIGN,
    MODE_TUTORIAL
} GameMode;


extern ButtonText GAME_MODES[];


typedef struct {
    sfTexture** textures;
    sfSoundBuffer** sounds;
    ComponentData* components;
    ColliderGrid* grid;
    float ambient_light;
    int seed;
    int camera;
    int menu_camera;
    sfRenderTexture* light_texture;
    sfSprite* light_sprite;
    sfRenderTexture* shadow_texture;
    sfSprite* shadow_sprite;
    sfVideoMode mode;
    ButtonText map_name;
    GameMode game_mode;
} GameData;


// Globals
extern GameState game_state;
extern GameData* game_data;
extern sfRenderWindow* game_window;


void change_state_win();

void create_game(sfVideoMode mode);

void resize_game(GameData* data, sfVideoMode mode);

void start_game(GameData* data, Filename map_name);

void end_game(GameData* data);

void reset_game(GameData data);

void update_game(GameData data, float time_step);

void update_game_mode(GameData data, float time_step);

void draw_game(GameData data, sfRenderWindow* window);

void draw_debug(GameData data, sfRenderWindow* window, int debug_level);

void update_game_over(GameData data, sfRenderWindow* window, float time_step);

void draw_game_over(GameData data, sfRenderWindow* window);

void draw_game_mode(GameData data, sfRenderWindow* window);

int create_tutorial(ComponentData* components, sfVector2f position);

int create_level_end(ComponentData* components, sfVector2f position, float angle, float width, float height);

void draw_tutorials(sfRenderWindow* window, GameData data);
