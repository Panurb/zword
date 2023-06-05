#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "perlin.h"


#define PIXELS_PER_UNIT 128


typedef sfTexture* TextureArray[100];

void create_decal(ComponentData* components, sfVector2f pos, Filename filename);

sfTexture** load_textures();

int texture_index(Filename filename);

void set_texture(ImageComponent* image, TextureArray textures);

sfTexture* load_texture(Filename filename);

sfSprite* load_sprite(Filename filename, TextureArray textures);

void draw_ground(ComponentData* components, sfRenderWindow* window, int camera, TextureArray textures);

void draw(ComponentData* component, sfRenderWindow* window,  int camera, TextureArray textures);

void draw_outlines(ComponentData* component, sfRenderWindow* window,  int camera);

void draw_roofs(ComponentData* component, sfRenderWindow* window, int camera, TextureArray textures);

void change_texture(ComponentData* components, int entity, Filename filename, float width, float height);

void change_layer(ComponentData* components, int entity, Layer layer);

void create_noise(sfUint8* pixels, int width, int height, sfVector2f origin, sfColor color, float sharpness, Permutation p);

void create_noise_tileable(sfUint8* pixels, int width, int height, sfVector2f origin, sfColor color, Permutation p);

bool point_inside_image(ComponentData* components, int entity, sfVector2f point);
