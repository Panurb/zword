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
    "grass_tile",
    "pistol",
    "player",
    "wood_tile",
    "zombie",
    "zombie_dead"
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
        sfIntRect rect = {0, 0, image->width * PIXELS_PER_UNIT, image->height * PIXELS_PER_UNIT };
        sfSprite_setTextureRect(image->sprite, rect);
    }
}


void draw(ComponentData* component, sfRenderWindow* window, Camera* camera, TextureArray textures) {
    for (int j = 0; j < component->image.size; j++) {
        int i = component->image.order[j];

        ImageComponent* image = ImageComponent_get(component, i);

        if (!image->visible) continue;

        if (image->texture_changed) {
            set_texture(image, textures);
            image->texture_changed = false;
        }

        draw_sprite(window, camera, image->sprite, get_position(component, i), get_angle(component, i));
    }
}
