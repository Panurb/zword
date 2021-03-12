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


void play_sounds(ComponentData* components, sfRenderWindow* window, int camera, SoundArray sounds) {
    for (int i = 0; i < components->entities; i++) {
        SoundComponent* sound = SoundComponent_get(components, i);
        if (!sound) continue;

        for (int i; i < sound->size; i++) {
            SoundEvent* event = sound->events[i];
            if (event) {
                sfSound* sound = sfSound_create();
                int j = sound_index(event->filename);
                sfSound_setBuffer(sound, sounds[j]);
                sfSound_play(sound);
                // sound->events[i] = NULL;
            }
        }
    }
}
