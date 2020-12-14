#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>

#include "image.h"
#include "camera.h"
#include "util.h"
#include "component.h"
#include "perlin.h"
#include "particle.h"


static const char* IMAGES[] = {
    "blood",
    "blood_large",
    "board_tile",
    "brick_tile",
    "car",
    "fire",
    "flashlight",
    "grass_tile",
    "pistol",
    "player",
    "roof_tile",
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


void draw(ComponentData* components, sfRenderWindow* window, Camera* camera, TextureArray textures) {
    for (int j = 0; j < components->image.size; j++) {
        int i = components->image.order[j];

        ImageComponent* image = ImageComponent_get(components, i);

        if (image->layer == 6) break;

        if (image->texture_changed) {
            set_texture(image, textures);
            image->texture_changed = false;
        }

        if (image->alpha > 0.0) {
            if (image->outline > 0.0) {
                sfSprite_setColor(image->sprite, get_color(1.0, 1.0, 1.0, 1.0));
                sfVector2f scale = image->scale;
                scale.x *= (image->width + image->outline) / image->width;
                scale.y *= (image->height + image->outline) / image->height;
                draw_sprite(window, camera, image->sprite, get_position(components, i), get_angle(components, i), scale, 1);
            }
            sfSprite_setColor(image->sprite, get_color(1.0, 1.0, 1.0, image->alpha));
            draw_sprite(window, camera, image->sprite, get_position(components, i), get_angle(components, i), image->scale, 0);
        }

        draw_particles(components, window, camera, i);
    }
}


void draw_roofs(ComponentData* component, sfRenderWindow* window, Camera* camera, TextureArray textures) {
    for (int j = 0; j < component->image.size; j++) {
        int i = component->image.order[j];

        ImageComponent* image = ImageComponent_get(component, i);

        if (image->layer < 6) continue;

        if (image->texture_changed) {
            set_texture(image, textures);
            image->texture_changed = false;
        }

        if (image->alpha > 0.0) {
            draw_sprite(window, camera, image->sprite, get_position(component, i), get_angle(component, i), image->scale, 0);
        }
    }
}


void change_layer(ComponentData* components, int entity, int layer) {
    ImageComponent* image = ImageComponent_get(components, entity);

    if (layer < image->layer) {
        int i = find(entity, components->image.order, components->image.size) - 1;
        while (i >= 0 && components->image.array[components->image.order[i]]->layer > layer) {
            components->image.order[i + 1] = components->image.order[i];
            i--;
        }
        components->image.order[i + 1] = entity;
    } else {
        int i = find(entity, components->image.order, components->image.size) + 1;
        while (i < components->image.size && components->image.array[components->image.order[i]]->layer < layer) {
            components->image.order[i - 1] = components->image.order[i];
            i++;
        }
        components->image.order[i - 1] = entity;
    }

    image->layer = layer;
}


void create_noise(sfUint8* pixels, int width, int height, sfColor color, int permutation[512]) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float x = i / (float) PIXELS_PER_UNIT;
            float y = j / (float) PIXELS_PER_UNIT;
            float a = octave_perlin(x, y, 0.0, permutation, 4, 0.5);
            pixels[(i + j * width) * 4] = color.r;
            pixels[(i + j * width) * 4 + 1] = color.g;
            pixels[(i + j * width) * 4 + 2] = color.b;
            pixels[(i + j * width) * 4 + 3] = color.a * a;
        }
    }
}
