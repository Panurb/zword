#include <SFML/Audio.h>

#include "component.h"

// https://freesound.org/people/RSilveira_88/sounds/216316/
// https://freesound.org/people/willybilly1984/sounds/345335/
// https://freesound.org/people/FilmmakersManual/sounds/522510/
// https://freesound.org/people/theshaggyfreak/sounds/276271/
// https://freesound.org/people/mrickey13/sounds/515618/
// https://freesound.org/people/cookies+policy/sounds/555926/

#define MAX_SOUNDS 256

typedef sfSoundBuffer* SoundArray[100];

sfSoundBuffer** load_sounds();

void add_sound(ComponentData* components, int entity, Filename filename, float volume, float pitch);

void loop_sound(ComponentData* components, int entity, Filename filename, float volume, float pitch);

void stop_loop(ComponentData* components, int entity);

void play_sounds(ComponentData* components, int camera, SoundArray sounds, sfSound* channels[MAX_SOUNDS]);
