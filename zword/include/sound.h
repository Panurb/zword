#include <SFML/Audio.h>

#include "component.h"

// https://freesound.org/people/RSilveira_88/sounds/216316/
// https://freesound.org/people/willybilly1984/sounds/345335/

typedef sfSoundBuffer* SoundArray[100];

void load_sounds(SoundArray sounds);

void add_sound(ComponentData* components, int entity, Filename filename);

void play_sounds(ComponentData* components, sfRenderWindow* window, int camera, SoundArray sounds);
