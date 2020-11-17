#define _USE_MATH_DEFINES

#include <math.h>

#include <SFML/System/Vector2.h>


float norm(sfVector2f v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

float dist(sfVector2f a, sfVector2f b) {
    float x = a.x - b.x;
    float y = a.y - b.y;
    return sqrtf(x * x + y * y);
}

sfVector2f normalized(sfVector2f v) {
    float n = norm(v);
    sfVector2f vn = { v.x / n, v.y / n };
    return vn;
}

double to_degrees(double radians) {
    return radians * (180.0 / M_PI);
}

float dot(sfVector2f a, sfVector2f b) {
    return a.x * b.x + a.y * b.y;
}

sfVector2f polar_to_cartesian(float length, float angle) {
    sfVector2f v = { length * cosf(angle), length * sinf(angle) };
    return v;
}
