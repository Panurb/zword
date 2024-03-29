#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"
#include "perlin.h"


#define PIXELS_PER_UNIT 128


typedef sfTexture* TextureArray[100];

int create_decal(sfVector2f pos, Filename filename, float lifetime);

sfTexture** load_textures();

int texture_index(Filename filename);

void set_texture(ImageComponent* image);

sfTexture* load_texture(Filename filename);

void draw_ground(int camera);

void draw(int camera);

void draw_roofs(int camera);

void change_texture(int entity, Filename filename, float width, float height);

void change_layer(int entity, Layer layer);

void create_noise(sfUint8* pixels, int width, int height, sfVector2f origin, sfColor color, float sharpness, Permutation p);

void create_noise_tileable(sfUint8* pixels, int width, int height, sfVector2f origin, sfColor color, Permutation p);

bool point_inside_image(int entity, sfVector2f point);
