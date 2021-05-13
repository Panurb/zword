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
    "ammo_pistol",
    "ammo_rifle",
    "ammo_shotgun",
    "assault_rifle",
    "axe",
    "bench",
    "big_boy",
    "big_boy_dead",
    "blood",
    "blood_large",
    "board_tile",
    "brick_tile",
    "candle",
    "car",
    "fire",
    "flashlight",
    "gas",
    "grass_tile",
    "pistol",
    "player",
    "player_dead",
    "river_curve",
    "river_end",
    "river_tile",
    "road_curve",
    "road_end",
    "road_tile",
    "rock",
    "roof_tile",
    "shotgun",
    "stone_tile",
    "tiles_tile",
    "tree",
    "uranium",
    "wood_tile",
    "zombie",
    "zombie_dead"
};


sfTexture** load_textures() {
    int n = sizeof(IMAGES) / sizeof(IMAGES[0]);

    sfTexture** textures = malloc(sizeof(IMAGES) * sizeof(sfTexture*));

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

    return textures;
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

        // FIXME
        draw_road(components, window, camera, textures, i);

        float r = 2.0 * image->scale.x * sqrtf(image->width * image->width + image->height * image->height);
        if (!on_screen(components, camera, pos, r, r)) {
            continue;
        }

        if (image->texture_changed) {
            set_texture(image, textures);
            image->texture_changed = false;
        }

        if (image->alpha > 0.0f) {
            sfSprite_setColor(image->sprite, get_color(1.0, 1.0, 1.0, image->alpha));
            draw_sprite(window, components, camera, image->sprite, pos, get_angle(components, i), image->scale, 0);
        }

        draw_particles(components, window, camera, i);
    }
}


void draw_outlines(ComponentData* components, sfRenderWindow* window, int camera) {
    for (int j = 0; j < components->image.size; j++) {
        int i = components->image.order[j];
        ImageComponent* image = ImageComponent_get(components, i);
        if (image->layer == 7) break;

        sfVector2f pos = get_position(components, i);

        float r = 2.0 * image->scale.x * sqrtf(image->width * image->width + image->height * image->height);
        if (!on_screen(components, camera, pos, r, r)) {
            continue;
        }

        if (image->outline > 0.0f) {
            sfShader_setFloatUniform(CameraComponent_get(components, camera)->shaders[1], "offset", image->outline);
            draw_sprite(window, components, camera, image->sprite, pos, get_angle(components, i), image->scale, 1);
        }
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
            float x = origin.x + 4.0 * i / (float) PIXELS_PER_UNIT;
            float y = origin.y + 4.0 * (height - j) / (float) PIXELS_PER_UNIT;
            float a = octave_perlin(x, y, 0.0, p, 8, 4, 0.5);

            // a = smoothstep(a, 0.5, 50.0);

            color_pixel(pixels, width, i, j, color, a);
        }
    }
}
