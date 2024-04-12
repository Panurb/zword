#pragma once

#include <SDL.h>
#include <SDL_ttf.h>

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
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* shadow_texture;
    SDL_Texture* light_texture;
    SDL_GameController* controllers[4];
} App;


typedef struct {
    SDL_Texture** textures;
    TTF_Font* fonts[301];
} Resources;


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

extern Resources resources;
extern App app;


void change_state_win();

void load_resources();

void create_game(sfVideoMode mode);

void resize_game(sfVideoMode mode);

void start_game(Filename map_name);

void end_game();

void reset_game();

void update_game(float time_step);

void update_game_mode(float time_step);

void draw_game();

void draw_debug(int debug_level);

void update_game_over(float time_step);

void draw_game_over();

void draw_game_mode();

int create_tutorial(Vector2f position);

int create_level_end(Vector2f position, float angle, float width, float height);

void draw_tutorials();
