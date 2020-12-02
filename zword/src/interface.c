#include <stdlib.h>
#include <stdio.h>

#include <SFML/Graphics.h>

#include "interface.h"


FpsCounter* FpsCounter_create() {
    FpsCounter* fps = malloc(sizeof(FpsCounter));
    for (int i = 0; i < FRAME_WINDOW; i++) {
        fps->frame_times[i] = 1.0 / 60.0;
    }
    fps->frame_avg = 1.0 / 60.0;
    fps->iterator = 0;
    fps->text = sfText_create();

    sfFont* font = sfFont_createFromFile("data/Helvetica.ttf");
    if (!font) {
        printf("Font not found!");
    }

    sfText_setFont(fps->text, font);
    sfText_setCharacterSize(fps->text, 20);
    sfText_setColor(fps->text, sfWhite);

    return fps;
}


void draw_fps(sfRenderWindow* window, FpsCounter* fps, float delta_time) {
    fps->frame_avg -= fps->frame_times[fps->iterator] / FRAME_WINDOW;
    fps->frame_times[fps->iterator] = delta_time;
    fps->frame_avg += fps->frame_times[fps->iterator] / FRAME_WINDOW;

    fps->iterator = (fps->iterator + 1) % FRAME_WINDOW;

    char buffer[20];
    snprintf(buffer, 20, "%.0f", 1.0 / fps->frame_avg);
    sfText_setString(fps->text, buffer);

    sfRenderWindow_drawText(window, fps->text, NULL);
}
