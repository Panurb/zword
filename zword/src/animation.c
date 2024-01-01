#include <math.h>
#include <string.h>

#include "component.h"
#include "animation.h"
#include "image.h"


int animation_frames(Filename image) {
    if (strcmp(image, "arms_axe") == 0) {
        return 6;
    }  else if (strcmp(image, "arms_sword") == 0) {
        return 8;
    } else if (strcmp(image, "big_boy") == 0) {
        return 2;
    } else if (strcmp(image, "boss_body") == 0) {
        return 4;
    }

    return 1;
}


void animate(ComponentData* components, float time_step) {
    for (int i = 0; i < components->entities; i++) {
        AnimationComponent* animation = AnimationComponent_get(components, i);
        if (!animation) continue;

        PhysicsComponent* physics = PhysicsComponent_get(components, i);
        if (physics) {
            animation->framerate = physics->speed;
        }

        JointComponent* joint = JointComponent_get(components, i);
        if (joint) {
            animation->framerate = AnimationComponent_get(components, joint->parent)->framerate;
        }

        if (animation->framerate == 0.0f) continue;

        animation->timer += time_step;
        float frame_time = 1.0f / animation->framerate;
        if (animation->timer > frame_time) {
            animation->current_frame++;
            animation->timer -= frame_time;
        }
        if (animation->current_frame >= animation->frames) {
            animation->current_frame = 0;
            if (animation->play_once) {
                animation->framerate = 0.0f;
            }
        }

        ImageComponent* image = ImageComponent_get(components, i);
        int width = PIXELS_PER_UNIT * image->width;
        int height = PIXELS_PER_UNIT * image->height;
        sfIntRect rect = { animation->current_frame * width, 0, width, height };
        sfSprite_setTextureRect(image->sprite, rect);
    }
}


void stop_animation(ComponentData* components, int entity) {
    AnimationComponent* animation = AnimationComponent_get(components, entity);
    animation->current_frame = 0;
    animation->framerate = 0.0f;
    animation->timer = 0.0f;
}
