#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <SFML/Audio.h>
#include <SFML/Graphics.h>
#include <SFML/System/Vector2.h>
#include <SFML/Window/Keyboard.h>
#include <SFML/Audio/Music.h>

#include <SDL.h>
#include <SDL_image.h>

#include "sound.h"
#include "game.h"
#include "interface.h"
#include "menu.h"
#include "settings.h"
#include "hud.h"
#include "input.h"
#include "serialize.h"
#include "editor.h"
#include "widget.h"
#include "game.h"


static String version = "0.1";


void create_game_window(sfVideoMode* mode) {
    mode->width = game_settings.width;
    mode->height = game_settings.height;
    sfContext* context = sfContext_create();
    sfContextSettings settings = sfContext_getSettings(context);
    sfContext_destroy(context);
    settings.antialiasingLevel = game_settings.antialiasing;
    sfUint32 style = game_settings.fullscreen ? sfFullscreen : sfClose;
    sfRenderWindow* window = sfRenderWindow_create(*mode, "zword", style, &settings);
    sfRenderWindow_setKeyRepeatEnabled(window, false);
    sfRenderWindow_setMouseCursorVisible(window, false);
    sfWindow_setVerticalSyncEnabled((sfWindow*) window, game_settings.vsync);
    if (game_settings.max_fps) {
        sfWindow_setFramerateLimit((sfWindow*) window, game_settings.max_fps);
    }

    game_window = window;

    app.window = SDL_CreateWindow("NotK", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, game_settings.width, game_settings.height, SDL_WINDOW_SHOWN );
    app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);
    app.shadow_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.shadow_texture, SDL_BLENDMODE_BLEND);
    app.light_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.light_texture, SDL_BLENDMODE_BLEND);
}


int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    TTF_Init();
    
    load_settings();

    sfVideoMode mode = { game_settings.width, game_settings.height, 32 };
    create_game_window(&mode);

    bool focus = true;
    int debug_level = 0;
    sfJoystick_update();

    sfSound* channels[MAX_SOUNDS];
    for (int i = 0; i < MAX_SOUNDS; i++) {
        channels[i] = sfSound_create();
    }

    FpsCounter* fps = FpsCounter_create();

    sfClock* clock = sfClock_create();
    float time_step = 1.0f / 60.0f;
    float elapsed_time = 0.0f;

    sfClock* game_clock = sfClock_create();

    load_resources();
    create_game(mode);
    create_menu();

    sfMusic* music = sfMusic_createFromFile("data/music/zsong.ogg");
    bool music_playing = false;
    sfMusic_setLoop(music, true);
    float music_fade = 0.0f;

    float title_scale = 2.0f;

    while (sfRenderWindow_isOpen(game_window)) {
        float delta_time = sfTime_asSeconds(sfClock_restart(clock));

        sfEvent event;
        while (sfRenderWindow_pollEvent(game_window, &event)) {
            switch (event.type) {
                case sfEvtLostFocus:
                    focus = false;
                    break;
                case sfEvtGainedFocus:
                    focus = true;
                    sfClock_restart(clock);
                    break;
                case sfEvtClosed:
                    sfRenderWindow_close(game_window);
                    break;
                case sfEvtKeyPressed:
                    if (event.key.code == sfKeyEscape) {
                        if (game_state == STATE_GAME) {
                            game_state = STATE_PAUSE;
                        } else if (game_state == STATE_PAUSE) {
                            game_state = STATE_GAME;
                        }
                    } else if (event.key.code == sfKeyF1) {
                        if (game_settings.debug) {
                            debug_level = (debug_level + 1) % 4;
                        }
                    }
                default:
                    if (game_state == STATE_EDITOR) {
                        input_editor(event);
                    }
                    if (game_state == STATE_MENU || game_state == STATE_PAUSE || game_state == STATE_GAME_OVER) {
                        input_menu(game_data->menu_camera, event);
                    }
                    break;
            }
        }

        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event))
        {
            switch (sdl_event.type)
            {
                case SDL_QUIT:
                    exit(0);
                    break;

                default:
                    break;
            }
        }

        if (focus) {
            while (elapsed_time > time_step) {
                elapsed_time -= time_step;
                switch (game_state) {
                    case STATE_MENU:
                        update_menu();
                        break;
                    case STATE_START:
                        start_game(game_data->map_name);
                        game_state = STATE_GAME;
                        break;
                    case STATE_END:
                        end_game();
                        clear_sounds(channels);
                        game_state = STATE_MENU;
                        break;
                    case STATE_RESET:
                        end_game();
                        clear_sounds(channels);
                        game_state = STATE_START;
                        break;
                    case STATE_GAME:
                        input(game_data->camera);
                        update_game(time_step);
                        update_game_mode(time_step);
                        break;
                    case STATE_PAUSE:
                        update_menu(game_window);
                        break;
                    case STATE_APPLY:
                        if (game_settings.width != (int)mode.width || game_settings.height != (int)mode.height) {
                            sfRenderWindow_destroy(game_window);
                            create_game_window(&mode);
                            resize_game(mode);
                        }
                        game_state = STATE_MENU;
                        break;
                    case STATE_CREATE:
                        destroy_menu();
                        create_editor_menu();
                        game_state = STATE_EDITOR;
                        break;
                    case STATE_LOAD:
                        destroy_menu();
                        create_editor_menu();
                        load_game(game_data->map_name);
                        game_state = STATE_EDITOR;
                        break;
                    case STATE_EDITOR:
                        update_editor(time_step);
                        break;
                    case STATE_GAME_OVER:
                        update_game_over(time_step);
                        break;
                    case STATE_QUIT:
                        sfRenderWindow_close(game_window);
                        break;
                }
            }

            elapsed_time += delta_time;

            title_scale = 2.5f + 0.1f * sinf(2.0f * sfTime_asSeconds(sfClock_getElapsedTime(game_clock)));

            music_fade = fminf(1.0f, music_fade + 0.01f);
        }

        sfRenderWindow_clear(game_window, get_color(0.05f, 0.05f, 0.05f, 1.0f));
        
        SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
        SDL_RenderClear(app.renderer);

        switch (game_state) {
            case STATE_MENU:
                draw_sprite(game_data->menu_camera, "menu", 0, 0, 0, zeros(), 0.0f, mult(3.5f, ones()), 1.0f, 0);
                draw_sprite(game_data->menu_camera, "title", 0, 0, 0, vec(0.0f, 9.0f), 0.0f, vec(title_scale, title_scale), 1.0f, 0);
                draw_menu();

                String buffer;
                snprintf(buffer, STRING_SIZE, "v%s", version);
                sfColor color = get_color(1.0f, 1.0f, 0.0f, 0.5f);
                draw_text(game_data->menu_camera, NULL, vec(24.5f, -13.5f), buffer, 20, color);
                break;
            case STATE_START:
            case STATE_END:
            case STATE_RESET:
                draw_text(game_data->menu_camera, NULL, zeros(), "LOADING", 20, sfWhite);
                break;
            case STATE_GAME:
                draw_game();
                draw_hud(game_data->camera);
                draw_game_mode();
                break;
            case STATE_PAUSE:
                draw_game();
                draw_overlay(game_data->camera, 0.4f);
                draw_menu();
                break;
            case STATE_LOAD:
                draw_text(game_data->menu_camera, NULL, zeros(), "LOADING", 20, sfWhite);
                break;
            case STATE_EDITOR:
                draw_editor();
                draw_hud(game_data->camera);
                break;
            case STATE_GAME_OVER:
                draw_game_over();
                break;
            default:
                break;
        }

        if (debug_level) {
            draw_debug(debug_level);
        }

        draw_fps(game_window, fps, delta_time);

        sfRenderWindow_display(game_window);

        SDL_RenderPresent(app.renderer);

        play_sounds(game_data->camera, channels);
        sfMusic_setVolume(music, 0.5f * game_settings.music * music_fade);
        if (!music_playing) {
            sfMusic_play(music);
            sfMusic_setVolume(music, 0.0f);
            music_playing = true;
        }
    }

    sfRenderWindow_destroy(game_window);

    SDL_DestroyWindow(app.window);
    SDL_Quit();

    return 0;
}
