#include "component.h"
#include "collider.h"
#include "image.h"
#include "sound.h"
#include "globals.h"


typedef struct {
    sfTexture** textures;
    sfSoundBuffer** sounds;
    ComponentData* components;
    ColliderGrid* grid;
    float ambient_light;
    int seed;
    int camera;
    sfRenderTexture* light_texture;
    sfSprite* light_sprite;
    sfRenderTexture* shadow_texture;
    sfSprite* shadow_sprite;
    sfVideoMode mode;
} GameData;


GameData create_game(sfVideoMode mode);

void start_game(GameData data);

void reset_game(GameData data);

void update_game(GameData data, sfRenderWindow* window, float time_step);

void draw_game(GameData data, sfRenderWindow* window);
