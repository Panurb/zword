#pragma once

#include <stdbool.h>

#include <SFML/System/Vector2.h>
#include <SFML/Graphics.h>


#define PRINT(x) printf("%d\n", x);

#define COLOR_ENERGY get_color(0.5f, 1.0f, 0.0f, 1.0f)

#define UNUSED(x) (void)(x)

#define LENGTH(x) (int)(sizeof(x)/sizeof(x[0]))

typedef struct {
    float a;
    float b;
    float c;
    float d;
} Matrix2f;

#define STRING_SIZE 1024
typedef char String[STRING_SIZE];

typedef char Filename[128];

#define BUTTON_TEXT_SIZE 128
typedef char ButtonText[BUTTON_TEXT_SIZE];

sfVector2f zeros();

sfVector2f ones();

sfVector2f vec(float x, float y);

float norm(sfVector2f v);

float norm2(sfVector2f v);

float dist(sfVector2f a, sfVector2f b);

sfVector2f normalized(sfVector2f v);

double to_degrees(double radians);

float dot(sfVector2f a, sfVector2f b);

float signed_angle(sfVector2f a, sfVector2f b);

sfVector2f bisector(sfVector2f a, sfVector2f b);

sfVector2f polar_to_cartesian(float length, float angle);

int abs_argmin(float* a, int n);

int argmax(float* a, int n);

float mean(float* array, int size);

sfVector2f perp(sfVector2f v);

float sign(float x);

sfVector2f sum(sfVector2f v, sfVector2f u);

sfVector2f diff(sfVector2f v, sfVector2f u);

sfVector2f mult(float c, sfVector2f v);

sfVector2f proj(sfVector2f a, sfVector2f b);

sfVector2f lin_comb(float a, sfVector2f v, float b, sfVector2f u);

float randf(float min, float max);

int randi(int low, int upp);

float rand_angle();

sfVector2f rand_vector();

int rand_choice(float* probs, int size);

int find(int value, int* array, int size);

int replace(int old, int new, int* array, int size);

int mini(int a, int b);

int maxi(int a, int b);

float mod(float x, float y);

float cross(sfVector2f v, sfVector2f u);

sfVector2f rotate(sfVector2f v, float angle);

float polar_angle(sfVector2f v);

Matrix2f rotation_matrix(float angle);

sfVector2f matrix_mult(Matrix2f m, sfVector2f v);

Matrix2f transpose(Matrix2f m);

Matrix2f matrix_inverse(Matrix2f m);

sfColor get_color(float r, float g, float b, float a);

void permute(int* array, int size);

float lerp(float a, float b, float t);

float smoothstep(float x, float mu, float nu);

int binary_search_filename(Filename filename, char** array, int size);

bool non_zero(sfVector2f v);

float clamp(float val, float min_val, float max_val);

float angle_diff(float a, float b);

bool collides_aabb(sfVector2f pos1, float w1, float h1, sfVector2f pos2, float w2, float h2);

bool point_inside_rectangle(sfVector2f position, float angle, float width, float height, sfVector2f point);
