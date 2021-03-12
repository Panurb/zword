#include <SFML/Audio.h>

#include "component.h"

typedef sfSoundBuffer* SoundArray[100];

void load_sounds(SoundArray sounds);

void play_sounds(ComponentData* components, sfRenderWindow* window, int camera, SoundArray sounds);
