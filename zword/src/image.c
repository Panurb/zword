#include <stdio.h>

#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>

#include "camera.h"
#include "util.h"
#include "component.h"


sfSprite* load_sprite(char filename[20]) {
    char path[100];
    snprintf(path, 100, "%s%s%s", "data/images/", filename, ".png");

    sfTexture* texture = sfTexture_createFromFile(path, NULL);

    if (!texture) {
        printf("Invalid filename!");
    }

    sfSprite* sprite = sfSprite_create();
    sfSprite_setTexture(sprite, texture, sfTrue);

    return sprite;
}

void draw(Component* component, sfRenderWindow* window, Camera* camera) {
    for (int i = 0; i < component->entities; i++) {
        if (!component->image[i]) continue;

        sfSprite_setPosition(component->image[i]->sprite, world_to_screen(component->coordinate[i]->position, camera));

        sfVector2f scale = { component->image[i]->scale.x * camera->zoom / 100.0, component->image[i]->scale.y * camera->zoom / 100.0 };
        sfSprite_setScale(component->image[i]->sprite, scale);

        sfSprite_setRotation(component->image[i]->sprite, to_degrees(-component->coordinate[i]->angle));

        sfFloatRect gb = sfSprite_getLocalBounds(component->image[i]->sprite);
        sfVector2f origin = { 0.5 * gb.width, 0.5 * gb.height };
        sfSprite_setOrigin(component->image[i]->sprite, origin);

        sfRenderWindow_drawSprite(window, component->image[i]->sprite, NULL);
    }
}
