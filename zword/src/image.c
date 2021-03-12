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
#include "road.h"


static const char* IMAGES[] = {
    "blood",
    "blood_large",
    "board_tile",
    "brick_tile",
    "car",
    "fire",
    "flashlight",
    "gas",
    "grass_tile",
    "pistol",
    "player",
    "road_curve",
    "road_end",
    "road_tile",
    "rock",
    "roof_tile",
    "tree",
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
    return binary_search_filename(filename, IMAGES, sizeof(IMAGES) / sizeof(IMAGES[0]));
}


void set_texture(ImageComponent* image, TextureArray textures) {
    int i = texture_index(image->filename);
    
    if (i != -1) {
        sfSprite_setTexture(image->sprite, textures[i], sfTrue);
        sfIntRect rect = { 0, 0, image->width * PIXELS_PER_UNIT, image->height * PIXELS_PER_UNIT };
        sfSprite_setTextureRect(image->sprite, rect);
    }
}


void draw(ComponentData* components, sfRenderWindow* window, int camera, TextureArray textures) {
    for (int j = 0; j < components->image.size; j++) {
        int i = components->image.order[j];

        ImageComponent* image = ImageComponent_get(components, i);

        if (image->layer == 7) break;

        sfVector2f pos = get_position(components, i);

        float r = 2.0 * image->scale.x * sqrtf(image->width * image->width + image->height * image->height);
        if (!on_screen(components, camera, pos, r, r)) {
            continue;
        }

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
                draw_sprite(window, components, camera, image->sprite, pos, get_angle(components, i), scale, 1);
            }
            sfSprite_setColor(image->sprite, get_color(1.0, 1.0, 1.0, image->alpha));
            draw_sprite(window, components, camera, image->sprite, pos, get_angle(components, i), image->scale, 0);
        }

        draw_particles(components, window, camera, i);
        draw_road(components, window, camera, textures, i);
    }
}


void draw_roofs(ComponentData* components, sfRenderWindow* window, int camera, TextureArray textures) {
    for (int j = 0; j < components->image.size; j++) {
        int i = components->image.order[j];

        ImageComponent* image = ImageComponent_get(components, i);

        if (image->layer < 7) continue;

        if (image->texture_changed) {
            set_texture(image, textures);
            image->texture_changed = false;
        }

        if (image->alpha > 0.0) {
            draw_sprite(window, components, camera, image->sprite, get_position(components, i), get_angle(components, i), image->scale, 0);
        }
    }
}


void change_layer(ComponentData* components, int entity, int layer) {
    ImageComponent* image = ImageComponent_get(components, entity);

    if (layer < image->layer) {
        int i = find(entity, components->image.order, components->image.size) - 1;
        while (i >= 0 && ImageComponent_get(components, components->image.order[i])->layer > layer) {
            components->image.order[i + 1] = components->image.order[i];
            i--;
        }
        components->image.order[i + 1] = entity;
    } else {
        int i = find(entity, components->image.order, components->image.size) + 1;
        while (i < components->image.size && ImageComponent_get(components, components->image.order[i])->layer < layer) {
            components->image.order[i - 1] = components->image.order[i];
            i++;
        }
        components->image.order[i - 1] = entity;
    }

    image->layer = layer;
}


void color_pixel(sfUint8* pixels, int width, int x, int y, sfColor color, float alpha) {
    int k = (x + y * width) * 4;

    pixels[k] = color.r;
    pixels[k + 1] = color.g;
    pixels[k + 2] = color.b;
    pixels[k + 3] = color.a * alpha;
}


void create_noise(sfUint8* pixels, int width, int height, sfVector2f origin, sfColor color, Permutation p) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float x = origin.x + i / (float) PIXELS_PER_UNIT;
            float y = origin.y + (height - j) / (float) PIXELS_PER_UNIT;
            float a = octave_perlin(x, y, 0.0, p, 8, 4, 0.5);

            a = smoothstep(a, 0.5, 50.0);

            color_pixel(pixels, width, i, j, color, a);
        }
    }
}


void create_noise_tileable(sfUint8* pixels, int width, int height, sfVector2f origin, sfColor color, Permutation p) {
    for (int i = 0; i < width / 2; i++) {
        for (int j = 0; j < height / 2; j++) {
            float x = origin.x + i / (float) PIXELS_PER_UNIT;
            float y = origin.y + (height - j) / (float) PIXELS_PER_UNIT;
            float a = octave_perlin(x, y, 0.0, p, -1, 4, 0.5);

            a = smoothstep(a, 0.5, 50.0);

            color_pixel(pixels, width, i, j, color, a);
            color_pixel(pixels, width, width - 1 - i, j, color, a);
            color_pixel(pixels, width, i, height - 1 - j, color, a);
            color_pixel(pixels, width, width - 1 - i, height - 1 - j, color, a);
        }
    }
}
