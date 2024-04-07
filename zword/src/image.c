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
#include "animation.h"
#include "game.h"
#include "list.h"


static const char* IMAGES[] = {
    "altar_tile",
    "ammo_pistol",
    "ammo_rifle",
    "ammo_shotgun",
    "arms",
    "arms_assault_rifle",
    "arms_axe",
    "arms_combat_shotgun",
    "arms_pistol",
    "arms_shotgun",
    "arms_smg",
    "arms_sword",
    "assault_rifle",
    "axe",
    "bandage",
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
    "combat_shotgun",
    "desk",
    "desk_debris",
    "desk_destroyed",
    "door",
    "energy",
    "farmer",
    "farmer_dead",
    "fence_tile",
    "fire",
    "flashlight",
    "gas",
    "grass_tile",
    "hay_bale",
    "hinge",
    "hole",
    "lamp",
    "menu",
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
    "smg",
    "stone_tile",
    "stove",
    "sword",
    "table",
    "table_destroyed",
    "tiles_tile",
    "title",
    "toilet",
    "tree",
    "uranium",
    "water_tile",
    "wood_tile",
    "zombie",
    "zombie_dead"
};


int create_decal(Vector2f pos, Filename filename, float lifetime) {
    int i = create_entity();
    CoordinateComponent_add(i, pos, rand_angle())->lifetime = 60.0f;
    ImageComponent_add(i, filename, 0.0f, 0.0f, LAYER_DECALS);
    PhysicsComponent_add(i, 0.0f)->lifetime = lifetime;

    return i;
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


int get_texture_index(Filename filename) {
    return binary_search_filename(filename, (char**) IMAGES, sizeof(IMAGES) / sizeof(IMAGES[0]));
}


void set_texture(ImageComponent* image) {
    image->texture_index = get_texture_index(image->filename);
}


void draw_ground(int camera) {
    ListNode* node;
    FOREACH(node, game_data->components->image.order) {
        int i = node->value;

        ImageComponent* image = ImageComponent_get(i);

        if (image->layer > LAYER_DECALS) break;

        if (image->texture_changed) {
            set_texture(image);
            image->texture_changed = false;
        }

        Vector2f pos = get_position(i);
        float w = image->scale.x * image->width;
        float h = image->scale.y * image->height;
        float r = sqrtf(w * w + h * h);

        // TODO: check if on screen
        draw_road(game_data->components, game_window, camera, game_data->textures, i);

        if (!on_screen(camera, pos, r, r)) {
            continue;
        }

        if (image->alpha > 0.0f) {
            draw_sprite(camera, image->filename, image->width, image->height, 0, pos, get_angle(i), image->scale, image->alpha, 0);
        }
    }
}


void draw_image(int entity, int camera) {
    ImageComponent* image = ImageComponent_get(entity);

    if (image->texture_changed) {
        set_texture(image);
        image->texture_changed = false;
    }

    image->scale.x = 1.0f - image->stretch;
    image->scale.y = 1.0f + image->stretch;

    Vector2f pos = get_position(entity);
    float w = image->scale.x * image->width;
    float h = image->scale.y * image->height;
    float r = sqrtf(w * w + h * h);
    if (!on_screen(camera, pos, r, r)) {
        return;
    }
    
    int offset = 0;
    AnimationComponent* animation = AnimationComponent_get(entity);
    if (animation) {
        offset = animation->current_frame;
    }

    if (image->alpha > 0.0f) {
        draw_sprite(camera, image->filename, image->width, image->height, offset, pos, get_angle(entity), image->scale, image->alpha, SHADER_NONE);
    }
}


void draw(int camera) {
    ListNode* node;
    FOREACH(node, game_data->components->image.order) {
        int i = node->value;

        ImageComponent* image = ImageComponent_get(i);

        if (image->layer <= LAYER_DECALS) continue;
        if (image->layer >= LAYER_ROOFS) break;

        draw_image(i, camera);
    }

    FOREACH(node, game_data->components->image.order) {
        draw_particles(camera, node->value);
    }
}


void draw_roofs(int camera) {
    ListNode* node;
    FOREACH(node, game_data->components->image.order) {
        int i = node->value;

        ImageComponent* image = ImageComponent_get(i);

        if (image->layer < LAYER_ROOFS) continue;

        draw_image(i, camera);
    }
}


void change_texture(int entity, Filename filename, float width, float height) {
    ImageComponent* image = ImageComponent_get(entity);
    if (strcmp(image->filename, filename) == 0) {
        return;
    }
    strcpy(image->filename, filename);
    image->width = width;
    image->height = height;
    image->texture_changed = true;
    if (filename[0] == '\0') {
        image->alpha = 0.0f;
    } else {
        image->alpha = 1.0f;
    }

    AnimationComponent* animation = AnimationComponent_get(entity);
    if (animation) {
        animation->frames = animation_frames(filename);
    }
}


void change_layer(int entity, Layer layer) {
    List* image_order = game_data->components->image.order;
    List_remove(image_order, entity);

    if (image_order->size == 0 || ImageComponent_get(image_order->head->value)->layer > layer) {
        List_add(image_order, entity);
    } else {
        ListNode* node;
        FOREACH(node, image_order) {
            if (!node->next || ImageComponent_get(node->next->value)->layer > layer) {
                List_insert(image_order, node, entity);
                break;
            }
        }
    }

    ImageComponent_get(entity)->layer = layer;
}


void color_pixel(sfUint8* pixels, int width, int x, int y, sfColor color, float alpha) {
    int k = (x + y * width) * 4;

    pixels[k] = color.r;
    pixels[k + 1] = color.g;
    pixels[k + 2] = color.b;
    pixels[k + 3] = color.a * alpha;
}


void create_noise(sfUint8* pixels, int width, int height, Vector2f origin, sfColor color, float sharpness, Permutation p) {
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


bool point_inside_image(int entity, Vector2f point) {
    Vector2f position = get_position(entity);
    float angle = get_angle(entity);
    ImageComponent* image = ImageComponent_get(entity);
    return point_inside_rectangle(position, angle, image->width, image->height, point);
}
