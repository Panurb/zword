#include <SFML/Audio.h>

#include "sound.h"


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
