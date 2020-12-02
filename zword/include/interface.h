#pragma once

#include <SFML/Graphics.h>


#define FRAME_WINDOW 100


typedef struct {
    float frame_times[FRAME_WINDOW];
    float frame_avg;
    int iterator;
    sfText* text;
} FpsCounter;

FpsCounter* FpsCounter_create();

void draw_fps(sfRenderWindow* window, FpsCounter* counter, float delta_time);
