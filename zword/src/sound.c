#include <stdio.h>
#include <string.h>

#include <SFML/Audio.h>

#include "sound.h"
#include "util.h"
#include "component.h"


static const char* SOUNDS[] = {
    "pistol"
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


void add_sound(ComponentData* components, int entity, Filename filename) {
    SoundComponent* scomp = SoundComponent_get(components, entity);
    for (int i = 0; i < scomp->size; i++) {
        SoundEvent event = scomp->events[i];
        if (event.filename[0] == '\0') {
            if (sfSound_getStatus(event.sound) == sfStopped) {
                strcpy(scomp->events[i].filename, filename);
                break;
            }
        }
    }
}


void play_sounds(ComponentData* components, sfRenderWindow* window, int camera, SoundArray sounds) {
    for (int i = 0; i < components->entities; i++) {
        SoundComponent* scomp = SoundComponent_get(components, i);
        if (!scomp) continue;

        for (int i = 0; i < scomp->size; i++) {
            SoundEvent event = scomp->events[i];
            if (event.filename[0] != '\0') {
                int j = sound_index(event.filename);
                sfSound_setBuffer(event.sound, sounds[j]);
                sfSound_play(event.sound);
                strcpy(scomp->events[i].filename, "");
            }
        }
    }
}
