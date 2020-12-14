#pragma once


void init_perlin(int p[512]);

float perlin(float x, float y, float z, int p[512]);

float octave_perlin(float x, float y, float z, int p[512], int octaves, float persistence);
