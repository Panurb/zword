#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "perlin.h"


#define PIXELS_PER_UNIT 128


typedef sfTexture* TextureArray[100];

void load_textures(TextureArray textures);

int texture_index(Filename filename);

void set_texture(ImageComponent* image, TextureArray textures);

sfTexture* load_texture(Filename filename);

sfSprite* load_sprite(Filename filename, TextureArray textures);

void draw(ComponentData* component, sfRenderWindow* window,  Camera* camera, TextureArray textures);

void draw_roofs(ComponentData* component, sfRenderWindow* window, Camera* camera, TextureArray textures);

void change_layer(ComponentData* components, int entity, int layer);

void create_noise(sfUint8* pixels, int width, int height, sfVector2f origin, sfColor color, Permutation p);

void create_noise_tileable(sfUint8* pixels, int width, int height, sfVector2f origin, sfColor color, Permutation p);
