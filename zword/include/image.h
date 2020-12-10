#pragma once

#include <SFML/Graphics.h>

#include "component.h"
#include "camera.h"


typedef sfTexture* TextureArray[100];

void load_textures(TextureArray textures);

int texture_index(Filename filename);

void set_texture(ImageComponent* image, TextureArray textures);

sfTexture* load_texture(Filename filename);

sfSprite* load_sprite(Filename filename, TextureArray textures);

void draw(ComponentData* component, sfRenderWindow* window,  Camera* camera, TextureArray textures);
