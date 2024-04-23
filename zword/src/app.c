#include <stdio.h>

#include <SDL.h>
#include <SDL_image.h>

#include "app.h"
#include "game.h"
#include "settings.h"
#include "interface.h"
#include "player.h"


App app;

static String version = "0.1";

static float title_scale = 2.0f;

static int debug_level = 0;

static FpsCounter* fps = NULL;


void create_game_window() {
    app.window = SDL_CreateWindow("NotK", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        game_settings.width, game_settings.height, SDL_WINDOW_SHOWN);
    SDL_SetWindowFullscreen(app.window, game_settings.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, game_settings.vsync ? "1" : "0");
    app.renderer = SDL_CreateRenderer(app.window, -1, SDL_RENDERER_ACCELERATED);

    app.shadow_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
        game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.shadow_texture, SDL_BLENDMODE_BLEND);

    app.light_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
        game_settings.width, game_settings.height);
    SDL_SetTextureBlendMode(app.light_texture, SDL_BLENDMODE_MUL);
}


void destroy_game_window() {
    SDL_DestroyWindow(app.window);
    app.window = NULL;
    SDL_DestroyRenderer(app.renderer);
    app.renderer = NULL;
    SDL_DestroyTexture(app.shadow_texture);
    app.shadow_texture = NULL;
    SDL_DestroyTexture(app.light_texture);
    app.light_texture = NULL;
}


void resize_game_window() {
    int w;
    int h;
    SDL_GetWindowSize(app.window, &w, &h);
    if (game_settings.width != w || game_settings.height != h) {
        SDL_SetWindowSize(app.window, game_settings.width, game_settings.height);
        SDL_SetWindowFullscreen(app.window, game_settings.fullscreen ? SDL_WINDOW_FULLSCREEN : 0);

        SDL_DestroyTexture(app.shadow_texture);
        app.shadow_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
            game_settings.width, game_settings.height);
        SDL_SetTextureBlendMode(app.shadow_texture, SDL_BLENDMODE_BLEND);

        SDL_DestroyTexture(app.light_texture);
        app.light_texture = SDL_CreateTexture(app.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 
            game_settings.width, game_settings.height);
        SDL_SetTextureBlendMode(app.light_texture, SDL_BLENDMODE_MUL);
    }
}


void init() {
    setbuf(stdout, NULL);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
    SDL_ShowCursor(SDL_DISABLE);
    
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    for (int i = 0; i < 4; i++) {
        app.controllers[i] = NULL;
    }

    create_game_window();

    fps = FpsCounter_create();

    app.quit = false;
    app.focus = true;
}


void quit() {
    free(fps);
    destroy_game_window();

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}


void input() {
    SDL_Event sdl_event;
    while (SDL_PollEvent(&sdl_event))
    {
        switch (sdl_event.type) {
            case SDL_WINDOWEVENT:
                if (sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                    app.focus = false;
                } else if (sdl_event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    app.focus = true;
                }
                break;
            case SDL_QUIT:
                app.quit = true;
                return;
            default:
                if (game_state == STATE_GAME) {
                    if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.repeat == 0) {
                        if (sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                            game_state = STATE_PAUSE;
                        } else if (sdl_event.key.keysym.sym == SDLK_F1) {
                            if (game_settings.debug) {
                                debug_level = (debug_level + 1) % 4;
                            }
                        }
                    }
                } else if (game_state == STATE_PAUSE) {
                    if (sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == SDLK_ESCAPE) {
                        game_state = STATE_GAME;
                    }
                }
                if (game_state == STATE_EDITOR) {
                    input_editor(sdl_event);
                }
                if (game_state == STATE_MENU || game_state == STATE_PAUSE || game_state == STATE_GAME_OVER) {
                    input_menu(game_data->menu_camera, sdl_event);
                }
                break;
        }
    }
}


void update(float time_step) {
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
            clear_sounds();
            game_state = STATE_MENU;
            break;
        case STATE_RESET:
            end_game();
            clear_sounds();
            game_state = STATE_START;
            break;
        case STATE_GAME:
            input_players(game_data->camera);
            update_game(time_step);
            update_game_mode(time_step);
            break;
        case STATE_PAUSE:
            update_menu();
            break;
        case STATE_APPLY:
            resize_game();
            resize_game_window();
            save_settings();
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
            app.quit = true;
            return;
    }

    float time = SDL_GetTicks() / 1000.0f;
    title_scale = 2.5f + 0.1f * sinf(2.0f * time);
}


void draw() {
    SDL_SetRenderDrawBlendMode(app.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(app.renderer, 0, 0, 0, 255);
    SDL_RenderClear(app.renderer);

    switch (game_state) {
        case STATE_MENU:
            draw_sprite(game_data->menu_camera, get_texture_index("menu"), 0, 0, 0, zeros(), 0.0f, mult(3.5f, ones()), 1.0f);
            draw_sprite(game_data->menu_camera, get_texture_index("title"), 0, 0, 0, vec(0.0f, 9.0f), 0.0f, vec(title_scale, title_scale), 1.0f);
            draw_menu();

            String buffer;
            snprintf(buffer, STRING_SIZE, "v%s", version);
            Color color = get_color(1.0f, 1.0f, 0.0f, 0.5f);
            draw_text(game_data->menu_camera, vec(24.5f, -13.5f), buffer, 20, color);
            break;
        case STATE_START:
        case STATE_END:
        case STATE_RESET:
            draw_text(game_data->menu_camera, zeros(), "LOADING", 20, COLOR_WHITE);
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
            draw_text(game_data->menu_camera, zeros(), "LOADING", 20, COLOR_WHITE);
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

    // FPSCounter_draw(fps, delta_time);

    SDL_RenderPresent(app.renderer);
}


void play_audio() {
    static bool music_playing = false;
    static float music_fade = 0.0f;

    music_fade = fminf(1.0f, music_fade + 0.01f);

    play_sounds(game_data->camera);
    Mix_VolumeMusic(0.5f * game_settings.music * music_fade);
    if (!music_playing) {
        Mix_VolumeMusic(0);
        Mix_PlayMusic(resources.music[0], -1);
        music_playing = true;
    }
}
