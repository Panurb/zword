#include "game.h"
#include "particle.h"


void init_weather() {
    if (game_data->weather) {
        if (game_data->weather == WEATHER_RAIN) {
            ParticleComponent* part = ParticleComponent_add_type(game_data->camera, PARTICLE_RAIN, 0.0f);
            SoundComponent* sound = SoundComponent_add(game_data->camera, "");
            strcpy(sound->loop_sound, "rain");
        } else if (game_data->weather == WEATHER_SNOW) {
            ParticleComponent* part = ParticleComponent_add_type(game_data->camera, PARTICLE_SNOW, 0.0f);
        }
        ParticleComponent* part = ParticleComponent_get(game_data->camera);
        Vector2f size = camera_size(game_data->camera);
        part->width = size.x;
        part->height = size.y;
    }

    game_data->wind = mult(game_data->wind_speed, rand_vector());
}


void update_weather(float time_step) {
    float wind_angle = polar_angle(game_data->wind);
    wind_angle += randf(-1.0f, 1.0f) * time_step;
    
    float wind_speed = norm(game_data->wind);
    wind_speed += randf(-1.0f, 1.0f) * time_step;
    wind_speed = clamp(wind_speed, 0.0f, game_data->wind_speed);

    game_data->wind = polar_to_cartesian(wind_speed, wind_angle);
}
