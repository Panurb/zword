#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>

#include "image.h"
#include "camera.h"
#include "util.h"
#include "component.h"


static const char* IMAGES[] = {
    "blood",
    "board_tile",
    "brick_tile",
    "car",
    "fire",
    "flashlight",
    "gradient",
    "grass_tile",
    "pistol",
    "player",
    "zombie"
};


void load_textures(TextureArray textures) {
    int n = sizeof(IMAGES) / sizeof(IMAGES[0]);
    for (int i = 0; i < n; i++) {
        char path[100];
        snprintf(path, 100, "%s%s%s", "data/images/", IMAGES[i], ".png");

        sfTexture* texture = sfTexture_createFromFile(path, NULL);
        sfTexture_setSmooth(texture, sfTrue);

        if (strstr(IMAGES[i], "tile")) {
            sfTexture_setRepeated(texture, sfTrue);
        }

        textures[i] = texture;
    }
}


int texture_index(Filename filename) {
    int l = 0;
    int r = sizeof(IMAGES) / sizeof(IMAGES[0]) - 1;

    while (l <= r) {
        int m = floor((l + r) / 2);

        if (strcmp(IMAGES[m], filename) < 0) {
            l = m + 1;
        } else if (strcmp(IMAGES[m], filename) > 0) {
            r = m - 1;
        } else {
            return m;
        }
    }

    return -1;
}


void set_texture(ImageComponent* image, TextureArray textures) {
    int i = texture_index(image->filename);
    
    if (i != -1) {
        sfSprite_setTexture(image->sprite, textures[i], sfTrue);
        sfSprite_setTextureRect(image->sprite, (sfIntRect) {0, 0, image->width * 128, image->height * 128 });
    }
}


void draw(ComponentData* component, sfRenderWindow* window, Camera* camera, TextureArray textures) {
    for (int i = 0; i < component->entities; i++) {
        ImageComponent* image = component->image[i];
        if (!image) continue;

        set_texture(image, textures);

        sfSprite_setPosition(component->image[i]->sprite, world_to_screen(get_position(component, i), camera));

        sfVector2f scale = { camera->zoom / 128.0, camera->zoom / 128.0 };
        sfSprite_setScale(component->image[i]->sprite, scale);

        sfSprite_setRotation(component->image[i]->sprite, to_degrees(-component->coordinate[i]->angle));

        sfFloatRect gb = sfSprite_getLocalBounds(component->image[i]->sprite);
        sfVector2f origin = { 0.5 * gb.width, 0.5 * gb.height };
        sfSprite_setOrigin(component->image[i]->sprite, origin);

        sfRenderWindow_drawSprite(window, component->image[i]->sprite, NULL);
    }
}
