#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <SFML/Graphics.h>

#include "settings.h"


Settings game_settings = { 1920, 1080, 8, 0, 0, 0, 100, 100 };


KeyValue parse_line(Line string, char* delim) {
    char* token = strtok(string, delim);
    char* key = token;
    token = strtok(NULL, delim);
    int value = strtol(token, NULL, 10);

    return (KeyValue) { key, value };
}


void load_settings() {
    FILE* file = fopen("config.cfg", "r");

    if (!file) {
        return;
    }

    Line buffer;
    while (fgets(buffer, 255, file)) {
        KeyValue line = parse_line(buffer, "=");

        printf("%s = %i\n", line.key, line.value);

        if (strcmp(line.key, "WIDTH") == 0) {
            game_settings.width = line.value;
        } else if (strcmp(line.key, "HEIGHT") == 0) {
            game_settings.height = line.value;
        } else if (strcmp(line.key, "ANTIALIASING") == 0) {
            game_settings.antialiasing = line.value;
        } else if (strcmp(line.key, "FULLSCREEN") == 0) {
            game_settings.fullscreen = line.value;
        } else if (strcmp(line.key, "VSYNC") == 0) {
            game_settings.vsync = line.value;
        } else if (strcmp(line.key, "MAX_FPS") == 0) {
            game_settings.max_fps = line.value;
        } else if (strcmp(line.key, "VOLUME") == 0) {
            game_settings.volume = line.value;
        } else if (strcmp(line.key, "MUSIC") == 0) {
            game_settings.music = line.value;
        }
    }

    fclose(file);
}


void save_settings() {
    FILE* file = fopen("config.cfg", "w");

    fprintf(file, "WIDTH=%i\n", game_settings.width);
    fprintf(file, "HEIGHT=%i\n", game_settings.height);
    fprintf(file, "ANTIALIASING=%i\n", game_settings.antialiasing);
    fprintf(file, "FULLSCREEN=%i\n", game_settings.fullscreen);
    fprintf(file, "VSYNC=%i\n", game_settings.vsync);
    fprintf(file, "MAX_FPS=%i\n", game_settings.max_fps);
    fprintf(file, "VOLUME=%i\n", game_settings.volume);
    fprintf(file, "MUSIC=%i\n", game_settings.music);

    fclose(file);
}
