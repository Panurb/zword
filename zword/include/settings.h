#pragma once


typedef struct {
    int width;
    int height;
    int antialiasing;
    bool fullscreen;
    bool vsync;
    int max_fps;
    int volume;
    int music;
} Settings;

typedef struct {
    char* key;
    int value;
} KeyValue;

typedef char Line[255];

extern Settings game_settings;

extern int keyboard_controls[12];

KeyValue parse_line(Line string, char* delim);

void load_settings();

void save_settings();
