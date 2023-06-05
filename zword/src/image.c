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
    "altar_tile",
    "ammo_pistol",
    "ammo_rifle",
    "ammo_shotgun",
    "arms",
    "arms_assault_rifle",
    "arms_pistol",
    "arms_shotgun",
    "assault_rifle",
    "axe",
    "beach_corner",
    "beach_tile",
    "bed",
    "bench",
    "bench_debris",
    "bench_destroyed",
    "big_boy",
    "big_boy_dead",
    "blood",
    "blood_large",
    "board_tile",
    "boss_body",
    "boss_dead",
    "boss_head",
    "brick_tile",
    "candle",
    "car",
    "desk",
    "desk_debris",
    "desk_destroyed",
    "door",
    "energy",
    "farmer",
    "farmer_dead",
    "fire",
    "flashlight",
    "gas",
    "grass_tile",
    "hay_bale",
    "lamp",
    "outhouse",
    "pistol",
    "player",
    "player_dead",
    "priest",
    "priest_dead",
    "river_curve",
    "river_end",
    "river_tile",
    "road_curve",
    "road_end",
    "road_tile",
    "rock",
    "roof_tile",
    "rope",
    "shotgun",
    "sink",
    "stone_tile",
    "stove",
    "table",
    "table_destroyed",
    "tiles_tile",
    "toilet",
    "tree",
    "uranium",
    "water_tile",
    "wood_tile",
    "zombie",
    "zombie_dead"
};


void create_decal(ComponentData* components, sfVector2f pos, Filename filename) {
    int i = create_entity(components);
    CoordinateComponent_add(components, i, pos, rand_angle());
    ImageComponent_add(components, i, filename, 0.0f, 0.0f, LAYER_DECALS);
    PhysicsComponent_add(components, i, 0.0f)->lifetime = INFINITY;
}


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
    return binary_search_filename(filename, (char**) IMAGES, sizeof(IMAGES) / sizeof(IMAGES[0]));
}


void set_texture(ImageComponent* image, TextureArray textures) {
    int i = texture_index(image->filename);
    
    if (i != -1) {
        sfSprite_setTexture(image->sprite, textures[i], sfTrue);
        if (image->width != 0.0f && image->height != 0.0f) {
            sfIntRect rect = { 0, 0, image->width * PIXELS_PER_UNIT, image->height * PIXELS_PER_UNIT };
            sfSprite_setTextureRect(image->sprite, rect);
        } else {
            sfFloatRect bounds = sfSprite_getLocalBounds(image->sprite);
            image->width = bounds.width / PIXELS_PER_UNIT;
            image->height = bounds.height / PIXELS_PER_UNIT;
        }
    }
}


void draw_ground(ComponentData* components, sfRenderWindow* window, int camera, TextureArray textures) {
    for (ListNode* node = components->image.order->head; node; node = node->next) {
        int i = node->value;

        ImageComponent* image = ImageComponent_get(components, i);

        if (image->layer > LAYER_DECALS) break;

        if (image->texture_changed) {
            set_texture(image, textures);
            image->texture_changed = false;
        }

        sfVector2f pos = get_position(components, i);
        float w = image->scale.x * image->width;
        float h = image->scale.y * image->height;
        float r = sqrtf(w * w + h * h);

        // TODO: check if on screen
        draw_road(components, window, camera, textures, i);

        if (!on_screen(components, camera, pos, r, r)) {
            continue;
        }

        if (image->alpha > 0.0f) {
            sfSprite_setColor(image->sprite, get_color(1.0f, 1.0f, 1.0f, image->alpha));
            draw_sprite(window, components, camera, image->sprite, pos, get_angle(components, i), image->scale, 0);
        }
    }
}


void draw(ComponentData* components, sfRenderWindow* window, int camera, TextureArray textures) {
    for (ListNode* node = components->image.order->head; node; node = node->next) {
        int i = node->value;

        ImageComponent* image = ImageComponent_get(components, i);

        if (image->layer <= LAYER_DECALS) continue;
        if (image->layer >= LAYER_ROOFS) break;

        if (image->texture_changed) {
            set_texture(image, textures);
            image->texture_changed = false;
        }

        sfVector2f pos = get_position(components, i);
        float w = image->scale.x * image->width;
        float h = image->scale.y * image->height;
        float r = sqrtf(w * w + h * h);
        if (!on_screen(components, camera, pos, r, r)) {
            continue;
        }

        if (image->alpha > 0.0f) {
            sfSprite_setColor(image->sprite, get_color(1.0f, 1.0f, 1.0f, image->alpha));
            draw_sprite(window, components, camera, image->sprite, pos, get_angle(components, i), image->scale, 0);
        }
    }

    for (ListNode* node = components->image.order->head; node; node = node->next) {
        draw_particles(components, window, camera, node->value);
    }
}


void draw_outlines(ComponentData* components, sfRenderWindow* window, int camera) {
    for (ListNode* node = components->image.order->head; node; node = node->next) {
        int i = node->value;
        ImageComponent* image = ImageComponent_get(components, i);
        if (image->layer == LAYER_ROOFS) break;

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
    for (ListNode* node = components->image.order->head; node; node = node->next) {
        int i = node->value;

        ImageComponent* image = ImageComponent_get(components, i);

        if (image->layer < LAYER_ROOFS) continue;

        if (image->texture_changed) {
            set_texture(image, textures);
            image->texture_changed = false;
        }

        if (image->alpha > 0.0f) {
            draw_sprite(window, components, camera, image->sprite, get_position(components, i), get_angle(components, i), image->scale, 0);
        }
    }
}


void change_texture(ComponentData* components, int entity, Filename filename, float width, float height) {
    ImageComponent* image = ImageComponent_get(components, entity);
    strcpy(image->filename, filename);
    image->width = width;
    image->height = height;
    image->texture_changed = true;
    if (filename[0] == '\0') {
        image->alpha = 0.0f;
    } else {
        image->alpha = 1.0f;
    }
}


void change_layer(ComponentData* components, int entity, Layer layer) {
    List_remove(components->image.order, entity);

    if (components->image.order->size == 0 || ImageComponent_get(components, components->image.order->head->value)->layer > layer) {
        List_add(components->image.order, entity);
    } else {
        for (ListNode* node = components->image.order->head; node; node = node->next) {
            if (!node->next || ImageComponent_get(components, node->next->value)->layer > layer) {
                List_insert(components->image.order, node, entity);
                break;
            }
        }
    }

    ImageComponent_get(components, entity)->layer = layer;
}


void color_pixel(sfUint8* pixels, int width, int x, int y, sfColor color, float alpha) {
    int k = (x + y * width) * 4;

    pixels[k] = color.r;
    pixels[k + 1] = color.g;
    pixels[k + 2] = color.b;
    pixels[k + 3] = color.a * alpha;
}


void create_noise(sfUint8* pixels, int width, int height, sfVector2f origin, sfColor color, float sharpness, Permutation p) {
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            float x = origin.x + 4.0 * i / (float) PIXELS_PER_UNIT;
            float y = origin.y + 4.0 * (height - j) / (float) PIXELS_PER_UNIT;
            float a = octave_perlin(x, y, 0.0, p, 8, 4, 0.5);

            if (sharpness != 0.0f) {
                a = smoothstep(a, 0.5f, 100.0f * sharpness);
            }

            color_pixel(pixels, width, i, j, color, a);
        }
    }
}


bool point_inside_image(ComponentData* components, int entity, sfVector2f point) {
    sfVector2f position = get_position(components, entity);
    float angle = get_angle(components, entity);
    ImageComponent* image = ImageComponent_get(components, entity);
    return point_inside_rectangle(position, angle, image->width, image->height, point);
}
