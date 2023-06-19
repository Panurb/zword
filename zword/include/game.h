#pragma once

#include "component.h"
#include "collider.h"
#include "image.h"
#include "sound.h"
#include "globals.h"


typedef enum {
    MODE_SURVIVAL
} GameMode;


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


GameData create_game(sfVideoMode mode);

void resize_game(GameData* data, sfVideoMode mode);

void start_game(GameData data);

void reset_game(GameData data);

void update_game(GameData data, sfRenderWindow* window, float time_step);

void draw_game(GameData data, sfRenderWindow* window);

void draw_debug(GameData data, sfRenderWindow* window, int debug_level);
