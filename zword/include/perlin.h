#pragma once


typedef int Permutation[512];

void init_perlin(Permutation p);

float perlin(float x, float y, float z, Permutation p);

float octave_perlin(float x, float y, float z, Permutation p, int octaves, float persistence);
