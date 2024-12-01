#include <stdio.h>
#include <string.h>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>

#include "app.h"
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
    "arms_sniper",
    "arms_sword",
    "arms_watergun",
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
    "birch",
    "blood",
    "blood_large",
    "blood_particle",
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
    "duckboard",
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
    "hollow_point",
    "lamp",
    "laser",
    "magazine",
    "marschall",
    "mat",
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
    "sauna",
    "scope",
    "shard",
    "shotgun",
    "silencer",
    "sink",
    "smg",
    "sniper",
    "spruce",
    "stone_tile",
    "stove",
    "swamp_tile",
    "sword",
    "table",
    "table_destroyed",
    "tiles_tile",
    "title",
    "toilet",
    "tree",
    "uranium",
    "water_tile",
    "watergun",
    "window",
    "window_destroyed",
    "wood_tile",
    "zombie",
    "zombie_dead"
};


float image_width(int entity) {
    Vector2f scale = get_scale(entity);
    ImageComponent* image = ImageComponent_get(entity);
    return scale.x * image->width;
}


float image_height(int entity) {
    Vector2f scale = get_scale(entity);
    ImageComponent* image = ImageComponent_get(entity);
    return scale.y * image->height;
}


Resolution get_texture_size(Filename filename) {
    if (filename[0] == '\0') {
        return (Resolution) { 0, 0 };
    }
    Resolution resolution;
    int index = get_texture_index(filename);
    SDL_QueryTexture(resources.textures[index], NULL, NULL, &resolution.w, &resolution.h);
    return resolution;
}


void set_pixel(SDL_Surface *surface, int x, int y, Color color) {
    const Uint32 pixel = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
    Uint32 * const target_pixel = (Uint32 *) ((Uint8 *) surface->pixels
                                                + y * surface->pitch
                                                + x * surface->format->BytesPerPixel);
    *target_pixel = pixel;
}


Color get_pixel(SDL_Surface *surface, int x, int y) {
    if (x < 0 || y < 0 || x >= surface->w || y >= surface->h) {
        return (Color) { 0, 0, 0, 0 };
    }

    Uint32 * const target_pixel = (Uint32 *) ((Uint8 *) surface->pixels
                                                + y * surface->pitch
                                                + x * surface->format->BytesPerPixel);
    Uint32 pixel = *target_pixel;
    Uint8 r, g, b, a;
    SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
    return (Color) { r, g, b, a };
}


int create_decal(Vector2f pos, Filename filename, float lifetime) {
    int i = create_entity();
    CoordinateComponent_add(i, pos, rand_angle())->lifetime = lifetime;
    ImageComponent_add(i, filename, 0.0f, 0.0f, LAYER_DECALS);

    return i;
}


SDL_Texture* create_outline_texture(Filename path) {
    SDL_Surface* surface = IMG_Load(path);

    // Create transparent surface
    SDL_Surface* outline_surface = SDL_CreateRGBSurface(0, surface->w, surface->h, 32, 
        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

    int offset = 5;
    for (int x = 0; x < surface->w; x++) {
        for (int y = 0; y < surface->h; y++) {
            if (get_pixel(surface, x, y).a > 0) {
                set_pixel(outline_surface, x, y, get_color(0.0f, 0.0f, 0.0f, 0.0f));
                continue;
            }

            int au = get_pixel(surface, x, y - offset).a;
            int ad = get_pixel(surface, x, y + offset).a;
            int al = get_pixel(surface, x - offset, y).a;
            int ar = get_pixel(surface, x + offset, y).a;

            if (au > 0 || ad > 0 || al > 0 || ar > 0) {
                set_pixel(outline_surface, x, y, COLOR_WHITE);
            }
        }
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(app.renderer, outline_surface);
    SDL_FreeSurface(outline_surface);
    SDL_FreeSurface(surface);

    return texture;
}


SDL_Texture* create_blood_particle_texture() {
    SDL_Surface* surface = SDL_CreateRGBSurface(0, PIXELS_PER_UNIT, PIXELS_PER_UNIT, 32, 
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    for (int i = 0; i < PIXELS_PER_UNIT; i++) {
        for (int j = 0; j < PIXELS_PER_UNIT; j++) {
            float x = 2.0f * i / (float) PIXELS_PER_UNIT - 1.0f;
            float y = 2.0f * j / (float) PIXELS_PER_UNIT - 1.0f;
            float r = sqrtf(x * x + y * y);

            if (r == 0.0f) {
                set_pixel(surface, i, j, get_color(1.0f, 0.0f, 0.0f, 1.0f));
            } else {
                float a = clamp(0.1f / r - 0.1f, 0.0f, 1.0f);
                set_pixel(surface, i, j, get_color(1.0f, 0.0f, 0.0f, a));
            }
        }
    }

    return SDL_CreateTextureFromSurface(app.renderer, surface);
}


void load_textures() {
    int n = sizeof(IMAGES) / sizeof(IMAGES[0]);

    SDL_Texture** textures = malloc(sizeof(IMAGES) * sizeof(SDL_Texture*));
    SDL_Texture** outline_textures = malloc(sizeof(IMAGES) * sizeof(SDL_Texture*));
    for (int i = 0; i < n; i++) {
        if (strcmp(IMAGES[i], "blood_particle") == 0) {
            textures[i] = create_blood_particle_texture();
            continue;
        }

        char path[100];
        snprintf(path, 100, "%s%s%s", "data/images/", IMAGES[i], ".png");

        SDL_Texture* texture = IMG_LoadTexture(app.renderer, path);

        textures[i] = texture;
        outline_textures[i] = create_outline_texture(path);
    }

    resources.textures = textures;
    resources.outline_textures = outline_textures;
}


int get_texture_index(Filename filename) {
    return binary_search_filename(filename, (char**) IMAGES, sizeof(IMAGES) / sizeof(IMAGES[0]));
}


void set_texture(ImageComponent* image) {
    image->texture_index = get_texture_index(image->filename);
}


void draw_image(int entity, int camera) {
    ImageComponent* image = ImageComponent_get(entity);
    if (image->alpha == 0.0f) {
        return;
    }

    Vector2f scale = get_scale_interpolated(entity, app.delta);
    // TODO: use delta
    float stretch_extrapolated = image->stretch + app.delta * app.time_step * image->stretch_speed;

    scale.x *= 1.0f - stretch_extrapolated;
    scale.y *= 1.0f + stretch_extrapolated;

    Vector2f pos = get_position_interpolated(entity, app.delta);
    float w = scale.x * image->width;
    float h = scale.y * image->height;
    float r = sqrtf(w * w + h * h);
    if (!on_screen(camera, pos, r, r)) {
        return;
    }
    
    int offset = 0;
    AnimationComponent* animation = AnimationComponent_get(entity);
    if (animation) {
        offset = animation->current_frame;
    }

    float angle = get_angle_interpolated(entity, app.delta);
    if (image->tile) {
        draw_tiles(camera, image->texture_index, w, h, pos, angle, image->alpha);
        return;
    }
    draw_sprite(camera, image->texture_index, image->width, image->height, offset, pos, angle, scale, image->alpha);
}


void draw_ground(int camera) {
    ListNode* node;
    FOREACH(node, game_data->components->image.order) {
        int i = node->value;

        ImageComponent* image = ImageComponent_get(i);
        if (image->layer > LAYER_DECALS) break;

        draw_image(i, camera);

        // TODO: check if on screen
        draw_road(camera, i);
    }
}


void draw_images(int camera) {
    ListNode* node;
    FOREACH(node, game_data->components->image.order) {
        int i = node->value;

        ImageComponent* image = ImageComponent_get(i);

        if (image->layer <= LAYER_DECALS) continue;
        if (image->layer >= LAYER_ROOFS) break;

        draw_image(i, camera);
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
    strcpy(image->filename, filename);
    image->width = width;
    image->height = height;

    if (width == 0.0f || height == 0.0f) {
        Resolution resolution = get_texture_size(filename);
        image->width = resolution.w / (float) PIXELS_PER_UNIT;
        image->height = resolution.h / (float) PIXELS_PER_UNIT;
    }
    
    if (filename[0] == '\0') {
        image->alpha = 0.0f;
    } else {
        image->alpha = 1.0f;
    }
    image->texture_index = get_texture_index(filename);

    if (strstr(filename, "tile") != NULL) {
        image->tile = true;
    } else {
        image->tile = false;
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


// void color_pixel(sfUint8* pixels, int width, int x, int y, Color color, float alpha) {
//     int k = (x + y * width) * 4;

//     pixels[k] = color.r;
//     pixels[k + 1] = color.g;
//     pixels[k + 2] = color.b;
//     pixels[k + 3] = color.a * alpha;
// }


// void create_noise(sfUint8* pixels, int width, int height, Vector2f origin, Color color, float sharpness, Permutation p) {
//     for (int i = 0; i < width; i++) {
//         for (int j = 0; j < height; j++) {
//             float x = origin.x + 4.0 * i / (float) PIXELS_PER_UNIT;
//             float y = origin.y + 4.0 * (height - j) / (float) PIXELS_PER_UNIT;
//             float a = octave_perlin(x, y, 0.0, p, 8, 4, 0.5);

//             if (sharpness != 0.0f) {
//                 a = smoothstep(a, 0.5f, 100.0f * sharpness);
//             }

//             color_pixel(pixels, width, i, j, color, a);
//         }
//     }
// }


bool point_inside_image(int entity, Vector2f point) {
    Vector2f position = get_position(entity);
    float angle = get_angle(entity);
    ImageComponent* image = ImageComponent_get(entity);
    return point_inside_rectangle(position, angle, image_width(entity), image_height(entity), point);
}
