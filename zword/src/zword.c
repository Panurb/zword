#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
#endif

#include "app.h"
#include "game.h"
#include "settings.h"
#include "menu.h"
#include "interface.h"
#include "editor.h"


static float time_step = 1.0f / 60.0f;
static float elapsed_time = 0.0f;


float get_delta_time() {
    static int start_time = 0;
    if (start_time == 0) {
        start_time = SDL_GetTicks();
    }
    int current_time = SDL_GetTicks();
    float delta_time = (current_time - start_time) / 1000.0f;
    start_time = current_time;
    return delta_time;
}


void main_loop() {
    float delta_time = get_delta_time();

    input();

    if (app.focus) {
        while (elapsed_time > time_step) {
            elapsed_time -= time_step;
            update(time_step);
        }

        elapsed_time += delta_time;
        FPSCounter_update(app.fps, delta_time);
    }
    
    draw();
    play_audio();
}


int main(int argc, char* argv[]) {
    load_settings();

    init();

    load_resources();
    create_game();
    create_menu();

    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop(main_loop, 0, true);
    #else
        while (!app.quit) {
            main_loop();
        }
    #endif

    quit();

    return 0;
}
