#include <math.h>

#include "component.h"
#include "animation.h"
#include "image.h"


void animate(ComponentData* components, float time_step) {
    for (int i = 0; i < components->entities; i++) {
        AnimationComponent* animation = AnimationComponent_get(components, i);
        if (!animation) continue;

        if (animation->framerate == 0.0f) continue;

        animation->timer += time_step;
        float frame_time = 1.0f / animation->framerate;
        if (animation->timer > frame_time) {
            animation->current_frame++;
            animation->timer -= frame_time;
        }
        if (animation->current_frame >= animation->frames) {
            animation->current_frame = 0;
        }

        ImageComponent* image = ImageComponent_get(components, i);
        int width = PIXELS_PER_UNIT * image->width;
        sfIntRect rect = { animation->current_frame * width, 0, width, image->height * PIXELS_PER_UNIT };
        sfSprite_setTextureRect(image->sprite, rect);
    }
}


void stop_animation(ComponentData* components, int entity) {
    AnimationComponent* animation = AnimationComponent_get(components, entity);
    animation->current_frame = 0;
    animation->framerate = 0.0f;
    animation->timer = 0.0f;
}
