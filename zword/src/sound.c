#define _USE_MATH_DEFINES

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <SFML/Audio.h>

#include "sound.h"
#include "util.h"
#include "component.h"


static const char* SOUNDS[] = {
    "car",
    "car_door",
    "metal",
    "pistol",
    "squish"
};


void load_sounds(SoundArray sounds) {
    int n = sizeof(SOUNDS) / sizeof(SOUNDS[0]);

    for (int i = 0; i < n; i++) {
        char path[100];
        snprintf(path, 100, "%s%s%s", "data/sfx/", SOUNDS[i], ".wav");

        sfSoundBuffer* sound = sfSoundBuffer_createFromFile(path);

        sounds[i] = sound;
    }
}


int sound_index(Filename filename) {
    return binary_search_filename(filename, SOUNDS, sizeof(SOUNDS) / sizeof(SOUNDS[0]));
}


void add_sound(ComponentData* components, int entity, Filename filename, float volume, float pitch) {
    SoundComponent* scomp = SoundComponent_get(components, entity);
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


void loop_sound(ComponentData* components, int entity, Filename filename, float volume, float pitch) {
    SoundComponent* scomp = SoundComponent_get(components, entity);
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


void stop_loop(ComponentData* components, int entity) {
    SoundComponent* scomp = SoundComponent_get(components, entity);
    for (int i = 0; i < scomp->size; i++) {
        SoundEvent* event = scomp->events[i];
        if (event) {
            if (event->loop) {
                event->loop = false;
            }
        }
    }
}


void play_sounds(ComponentData* components, int camera, SoundArray sounds, sfSound* channels[MAX_SOUNDS]) {
    // sfVector2f r = get_position(components, camera);
    // sfListener_setPosition((sfVector3f) { r.x, 0.0, r.y });

    for (int i = 0; i < components->entities; i++) {
        SoundComponent* scomp = SoundComponent_get(components, i);
        if (!scomp) continue;

        for (int j = 0; j < scomp->size; j++) {
            SoundEvent* event = scomp->events[j];
            if (!event) continue;

            int chan = event->channel;

            if (chan == -1) {
                for (int k = 0; k < MAX_SOUNDS; k++) {
                    if (sfSound_getStatus(channels[k]) != sfPlaying) {
                        chan = k;
                        break;
                    }
                }
            }

            sfSound* channel = channels[chan];

            // r = get_position(components, i);
            // sfSound_setPosition(channel, (sfVector3f) { r.x, 0.0, r.y });
            sfSound_setVolume(channel, event->volume * 100.0);
            sfSound_setPitch(channel, event->pitch);

            if (event->channel != -1) {
                if (!event->loop) {
                    event->volume *= 0.95;
                    if (event->volume < 0.01) {
                        sfSound_stop(channel);
                        free(event);
                        scomp->events[j] = NULL;
                    }
                }
            } else {
                sfSound_setBuffer(channel, sounds[sound_index(event->filename)]);
                sfSound_setLoop(channel, event->loop);
                sfSound_play(channel);

                if (event->loop) {
                    event->channel = chan;
                } else {
                    free(event);
                    scomp->events[j] = NULL;
                }
            }
        }
    }
}
