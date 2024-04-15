#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sound.h"
#include "util.h"
#include "component.h"
#include "settings.h"
#include "game.h"


static const char* SOUNDS[] = {
    "assault_rifle",
    "axe",
    "car",
    "car_door",
    "door",
    "energy",
    "fire",
    "geiger",
    "metal_hit",
    "pistol",
    "shotgun",
    "squish",
    "stone_hit",
    "wood_destroy",
    "wood_hit"
};


void load_sounds() {
    int n = sizeof(SOUNDS) / sizeof(SOUNDS[0]);

    for (int i = 0; i < n; i++) {
        char path[100];
        snprintf(path, 100, "%s%s%s", "data/sfx/", SOUNDS[i], ".wav");

        resources.sounds[i] = Mix_LoadWAV(path);
    }
}


int sound_index(Filename filename) {
    return binary_search_filename(filename, (char**) SOUNDS, sizeof(SOUNDS) / sizeof(SOUNDS[0]));
}


void add_sound(int entity, Filename filename, float volume, float pitch) {
    SoundComponent* scomp = SoundComponent_get(entity);
    for (int i = 0; i < scomp->size; i++) {
        if (!scomp->events[i]) {
            SoundEvent* event = malloc(sizeof(SoundEvent));
            strcpy(event->filename, filename);
            event->volume = volume;
            event->pitch = pitch;
            event->loop = false;
            event->channel = -1;
            scomp->events[i] = event;
            break;
        }
    }
}


void loop_sound(int entity, Filename filename, float volume, float pitch) {
    SoundComponent* scomp = SoundComponent_get(entity);
    for (int i = 0; i < scomp->size; i++) {
        if (!scomp->events[i]) {
            SoundEvent* event = malloc(sizeof(SoundEvent));
            strcpy(event->filename, filename);
            event->volume = volume;
            event->pitch = pitch;
            event->loop = true;
            event->channel = -1;
            scomp->events[i] = event;
            break;
        }
    }
}


void stop_loop(int entity) {
    SoundComponent* scomp = SoundComponent_get(entity);
    for (int i = 0; i < scomp->size; i++) {
        SoundEvent* event = scomp->events[i];
        if (event) {
            if (event->loop) {
                event->loop = false;
            }
        }
    }
}


void play_sounds(int camera) {
    for (int i = 0; i < game_data->components->entities; i++) {
        SoundComponent* scomp = SoundComponent_get(i);
        if (!scomp) continue;

        float dist = norm(diff(get_position(i), get_position(camera)));

        if (scomp->loop_sound[0] != '\0') {
            if (!scomp->events[0]) {
                loop_sound(i, scomp->loop_sound, 0.5f, 1.0f);
            }
        }

        for (int j = 0; j < scomp->size; j++) {
            SoundEvent* event = scomp->events[j];
            if (!event) continue;

            int chan = event->channel;

            float vol = event->volume;
            CameraComponent* cam = CameraComponent_get(camera);
            float radius = 0.5f * cam->resolution.x / cam->zoom;
            if (dist > radius) {
                vol = expf(-(dist - radius)) * event->volume;
            }

            if (event->loop) {
                vol = fmaxf(0.0f, (radius - dist) / radius) * event->volume;
            }

            if (chan != -1) {
                if (!event->loop) {
                    event->volume *= 0.95;
                    if (event->volume < 0.01) {
                        Mix_HaltChannel(chan);
                        free(event);
                        scomp->events[j] = NULL;
                    }
                }
            } else {
                int loops = event->loop ? -1 : 0;
                chan = Mix_PlayChannel(chan, resources.sounds[sound_index(event->filename)], loops);

                if (event->loop) {
                    event->channel = chan;
                } else {
                    free(event);
                    scomp->events[j] = NULL;
                }
            }

            Mix_Volume(chan, vol * MIX_MAX_VOLUME * game_settings.volume / 100.0f);
            // TODO: pitch
        }
    }
}


void clear_sounds() {
    Mix_HaltChannel(-1);
}
