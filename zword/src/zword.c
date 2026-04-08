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
#include "network.h"


static float elapsed_time = 0.0f;
static float time_since_last_update = 0.0f;


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
    time_since_last_update += delta_time;

    input();

    bool should_update = app.focus;
    if (network.mode != NET_MODE_NONE) {
        should_update = true;
    }

    if (should_update) {
        // Client doesn't simulate physics — it just applies snapshots.
        // Running the accumulator 2+ times per frame destroys interpolation state
        // (second tick has no new snapshot, so previous == current == no smoothing).
        // Cap to 1 update per frame and clamp accumulated time.
        if (network.mode == NET_MODE_CLIENT) {
            if (elapsed_time > app.time_step) {
                elapsed_time -= app.time_step;
                elapsed_time = fminf(elapsed_time, app.time_step);
                time_since_last_update = 0.0f;
                update(app.time_step);
            }
            elapsed_time += delta_time;
        } else {
            while (elapsed_time > app.time_step) {
                elapsed_time -= app.time_step;
                time_since_last_update = 0.0f;
                update(app.time_step);
            }

            elapsed_time += delta_time;
        }
        FPSCounter_update(app.fps, delta_time);
    }
    
    // 0 means only interpolate, 1 means only extrapolate
    float extrapolation_factor = 0.5f;

    app.delta = fminf(time_since_last_update / app.time_step, 1.0f) + extrapolation_factor;

    draw();
    play_audio();
}


int main(int argc, char* argv[]) {
    load_settings();

    app.argc = argc;
    app.argv = argv;

    init();

    load_resources();
    LOG_INFO("Resources loaded");
    create_game();
    LOG_INFO("Game created");

    #ifdef __EMSCRIPTEN__
        // Initialize filesystem, this is an async operation so need to create menu in the callback
        EM_ASM(
            FS.mkdir('/save');
            FS.mount(IDBFS, {}, '/save');
            FS.syncfs(true, function(err) {
                if (err) {
                    console.error('syncfs error:', err);
                } else {
                    console.log('syncfs complete');
                    ccall('create_menu', 'void', [], []);
                }
            });
        );
        emscripten_set_main_loop(main_loop, 0, true);
    #else
        create_menu();
        LOG_INFO("Menu created");

        while (!app.quit) {
            main_loop();
        }
    #endif

    quit();

    return 0;
}
