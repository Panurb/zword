#pragma once

#include <stdbool.h>

#include <SDL.h>

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* shadow_texture;
    SDL_Texture* light_texture;
    SDL_GameController* controllers[4];
    bool quit;
    bool focus;
} App;

extern App app;

void init();

void quit();

void input();

void update(float time_step);

void draw();

void play_audio();
