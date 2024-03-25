#pragma once

#include <SFML/Audio.h>

#include "component.h"

// https://freesound.org/people/RSilveira_88/sounds/216316/
// https://freesound.org/people/willybilly1984/sounds/345335/
// https://freesound.org/people/FilmmakersManual/sounds/522510/
// https://freesound.org/people/theshaggyfreak/sounds/276271/
// https://freesound.org/people/mrickey13/sounds/515618/
// https://freesound.org/people/cookies+policy/sounds/555926/
// https://freesound.org/people/ninebilly/sounds/520934/
// https://freesound.org/people/FilmmakersManual/sounds/522284/
// https://freesound.org/people/kwahmah_02/sounds/251365/
// https://freesound.org/people/Bertsz/sounds/500901/
// https://freesound.org/people/CamoMano/sounds/431019/
// https://freesound.org/people/aerror/sounds/350750/
// https://freesound.org/people/JarredGibb/sounds/219499/
// https://freesound.org/people/MCOcontentcreation/sounds/258289/
// https://freesound.org/people/aerror/sounds/350757/

// Max 256
#define MAX_SOUNDS 255

typedef sfSoundBuffer* SoundArray[100];

sfSoundBuffer** load_sounds();

void add_sound(int entity, Filename filename, float volume, float pitch);

void loop_sound(int entity, Filename filename, float volume, float pitch);

void stop_loop(int entity);

void play_sounds(int camera, sfSound* channels[MAX_SOUNDS]);

void clear_sounds(sfSound* channels[MAX_SOUNDS]);
