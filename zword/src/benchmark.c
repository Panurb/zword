#include <SDL.h>
#include <time.h>

#include "game.h"
#include "app.h"
#include "list.h"
#include "util.h"


float run_benchmark() {
    float time_step = 1.0f / 60.0f;
    int steps = 5000;

    srand(0);

    start_game("Survival", false);

    ListNode* node;
    FOREACH(node, game_data->components->player.order) {
        HealthComponent* health = HealthComponent_get(node->value);
        health->health = 100000;
    }

    float start_time = SDL_GetTicks();

    for (int i = 0; i < steps; i++) {
        update_game(time_step);
        update_game_mode(time_step);

        SDL_RenderClear(app.renderer);
        draw_game();
        SDL_RenderPresent(app.renderer);
    }
    srand(time(NULL));

    float total_time = (SDL_GetTicks() - start_time) / 1000.0f;

    end_game();

    float fps = steps / total_time;
    return fps;
}
