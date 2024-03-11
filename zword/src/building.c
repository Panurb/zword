#define _USE_MATH_DEFINES

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "component.h"
#include "particle.h"
#include "door.h"
#include "navigation.h"
#include "item.h"
#include "vehicle.h"
#include "image.h"
#include "enemy.h"
#include "object.h"
#include "tile.h"


void create_church(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = polar_to_cartesian(1.0f, angle + 0.5f * M_PI);

    create_floor(components, pos, 30.0f, 10.0f, angle, "tiles_tile");
    create_floor(components, sum(pos, mult(5.0f, w)), 10.0f, 20.0f, angle, "tiles_tile");

    create_door(sum(pos, lin_comb(-14.5f, w, 1.0f, h)), angle - 0.5f * M_PI);
    create_door(sum(pos, lin_comb(-14.5f, w, -1.0f, h)), angle + 0.5f * M_PI);

    // Back wall
    create_wall(components, sum(pos, mult(14.5f, w)), angle + 0.5f * M_PI, 8.0f, 1.0f, "stone_tile");

    // Altar
    create_wall(components, sum(pos, mult(12.5f, w)), angle, 3.0f, 8.0f, "altar_tile");
    create_candle(components, sum(pos, sum(mult(12.5f, w), mult(-3.0f, h))));
    create_candle(components, sum(pos, sum(mult(12.5f, w), mult(3.0f, h))));
    int k = create_entity();
    CoordinateComponent_add(k, sum(pos, mult(12.5f, w)), rand_angle());
    ImageComponent_add(k, "blood_large", 2.0f, 2.0f, LAYER_ITEMS);

    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < 7; i++) {
            create_bench(components, sum(pos, sum(mult((i - 5.75f) * 2.0f, w), mult(2.5f, h))), angle);
        }

        // Entrance
        create_wall(components, sum(pos, sum(mult(-14.5f, w), mult(3.0f, h))), angle + 0.5f * M_PI, 2.0f, 1.0f, "stone_tile");
        create_wall(components, sum(pos, sum(mult(-13.0f, w), mult(3.5f, h))), angle, 2.0f, 1.0f, "altar_tile");
        create_candle(components, sum(pos, sum(mult(-13.0f, w), mult(3.5f, h))));

        // Side wall entrance
        create_wall(components, sum(pos, sum(mult(-6.0f, w), mult(4.5f, h))), angle, 18.0f, 1.0f, "stone_tile");

        // Side wall back
        create_wall(components, sum(pos, sum(mult(11.0f, w), mult(4.5f, h))), angle, 8.0f, 1.0f, "stone_tile");
        
        // Side thingies
        create_wall(components, sum(pos, sum(mult(5.0f, w), mult(9.5f, h))), angle, 10.0f, 1.0f, "stone_tile");
        create_wall(components, sum(pos, sum(mult(0.5f, w), mult(7.0f, h))), angle + 0.5f * M_PI, 4.0f, 1.0f, "stone_tile");
        create_wall(components, sum(pos, sum(mult(9.5f, w), mult(7.0f, h))), angle + 0.5f * M_PI, 4.0f, 1.0f, "stone_tile");
        create_wall(components, sum(pos, sum(mult(5.0f, w), mult(8.5f, h))), angle, 2.0f, 1.0f, "altar_tile");
        create_candle(components, sum(pos, sum(mult(5.0f, w), mult(8.5f, h))));
        create_waypoint(sum(pos, sum(mult(5.0f, w), mult(5.0f, h))));

        // Outside
        create_waypoint(sum(pos, sum(mult(-16.0f, w), mult(6.0f, h))));
        create_waypoint(sum(pos, sum(mult(-8.0f, w), mult(6.0f, h))));
        create_waypoint(sum(pos, sum(mult(-1.0f, w), mult(11.0f, h))));
        create_waypoint(sum(pos, sum(mult(11.0f, w), mult(11.0f, h))));
        create_waypoint(sum(pos, sum(mult(16.0f, w), mult(6.0f, h))));

        h = mult(-1.0f, h);
    }

    create_waypoint(sum(pos, mult(-16.0f, w)));
    create_waypoint(sum(pos, mult(-12.0f, w)));
    create_waypoint(sum(pos, mult(2.0f, w)));
    create_waypoint(sum(pos, mult(8.0f, w)));

    int items = randi(1, 2);
    int locations[2] = {0, 1};
    permute(locations, 2);
    for (int i = 0; i < items; i++) {
        switch (locations[i]) {
            case 0:
                create_item(components, sum(pos, sum(mult(5.0f, w), mult(6.5f, h))), randi(2, 3));
                break;
            case 1:
                create_item(components, sum(pos, sum(mult(5.0f, w), mult(-6.5f, h))), randi(2, 3));
                break;
        }
    }
}


void create_barn(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = polar_to_cartesian(1.0f, angle + 0.5f * M_PI);

    create_floor(components, pos, 25.0f, 14.0f, angle, "board_tile");

    // Back wall
    create_wall(components, sum(pos, mult(12.0f, w)), angle + 0.5f * M_PI, 14.0f, 1.0f, "wood_tile");

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            sfVector2f r = sum(mult(3.0f * i - 4.5f, h), mult(6.5f + 2.0f * j, w));
            create_hay_bale(components, sum(pos, r), angle + 0.5f * M_PI + randf(-0.1f, 0.1f));
        }
    }

    for (int i = 0; i < 2; i++) {
        // Entrance
        create_wall(components, sum(pos, sum(mult(-12.0f, w), mult(4.5f, h))), angle + 0.5f * M_PI, 5.0f, 1.0f, "wood_tile");

        // Side
        create_wall(components, sum(pos, mult(6.5f, h)), angle, 23.0f, 1.0f, "wood_tile");

        // Middle pile
        create_hay_bale(components, sum(pos, sum(mult(-4.0f, w), mult(1.0f, h))), angle + randf(-0.1f, 0.1f));
        create_hay_bale(components, sum(pos, sum(mult(-1.0f, w), mult(1.0f, h))), angle + randf(-0.1f, 0.1f));
        create_waypoint(sum(pos, lin_comb(-6.5f, w, 3.0f, h)));
        create_waypoint(sum(pos, lin_comb(1.5f, w, 3.0f, h)));
        create_candle(components, sum(pos, lin_comb(-1.0f, w, 1.0f, h)));
        create_candle(components, sum(pos, lin_comb(-4.0f, w, 1.0f, h)));

        // Outside
        create_waypoint(sum(pos, lin_comb(-13.5f, w, 8.0f, h)));
        create_waypoint(sum(pos, lin_comb(0.0f, w, 8.0f, h)));
        create_waypoint(sum(pos, lin_comb(13.5f, w, 8.0f, h)));

        h = mult(-1.0f, h);
    }

    create_waypoint(sum(pos, mult(-11.0f, w)));
    create_waypoint(sum(pos, mult(-13.5f, w)));
    create_waypoint(sum(pos, mult(13.5f, w)));

    int items = randi(1, 3);
    int locations[3] = {0, 1, 2};
    permute(locations, 3);
    for (int i = 0; i < items; i++) {
        switch (locations[i]) {
            case 0:
                create_item(components, sum(pos, lin_comb(4.0f, w, 4.0f, h)), i);
                break;
            case 1:
                create_item(components, sum(pos, lin_comb(4.0f, w, -4.0f, h)), i);
                break;
            case 2:
                create_item(components, sum(pos, mult(4.0f, w)), i);
                break;
        }
    }
}


void create_house(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 16.0f, 16.0f, angle, "board_tile");
    // create_roof(components, sum(pos, mult(4.0f, h)), 16.0f, 8.0f, angle);
    // create_roof(components, sum(pos, mult(-4.0f, h)), 16.0f, 8.0f, angle + M_PI);

    // Outside walls
    create_wall(components, sum(pos, mult(-7.5f, w)), angle + 0.5 * M_PI, 16.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, mult(7.5f, h)), angle, 14.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, mult(-7.5f, h)), angle, 14.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(7.5f, w, 4.5f, h)), angle + 0.5 * M_PI, 7.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(7.5f, w, -4.5f, h)), angle + 0.5 * M_PI, 7.0f, 1.0f, "brick_tile");

    create_waypoint(sum(pos, lin_comb(9.0f, w, 9.0f, h)));
    create_waypoint(sum(pos, lin_comb(9.0f, w, -9.0f, h)));
    create_waypoint(sum(pos, lin_comb(-9.0f, w, 9.0f, h)));
    create_waypoint(sum(pos, lin_comb(-9.0f, w, -9.0f, h)));
    create_waypoint(sum(pos, lin_comb(-9.0f, w, 0.0f, h)));
    create_waypoint(sum(pos, lin_comb(0.0f, w, 9.0f, h)));
    create_waypoint(sum(pos, lin_comb(0.0f, w, -9.0f, h)));

    create_door(sum(pos, mult(7.5f, w)), angle + 0.5f * M_PI);

    // Toilet
    create_floor(components, sum(pos, lin_comb(-4.5f, w, 4.5f, h)), 6.0f, 6.0f, angle, "altar_tile");
    create_wall(components, sum(pos, lin_comb(-6.5f, w, 1.75f, h)), angle, 1.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, lin_comb(-1.75f, w, 4.5f, h)), angle + 0.5f * M_PI, 5.0f, 0.5f, "wood_tile");
    create_toilet(components, sum(pos, lin_comb(-6.0f, w, 6.0f, h)), angle + 1.5f * M_PI);
    create_sink(components, sum(pos, lin_comb(-3.5f, w, 6.0f, h)), angle + 1.5f * M_PI);

    // Bedroom
    create_wall(components, sum(pos, lin_comb(0.0f, w, 1.75f, h)), angle, 8.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, lin_comb(6.5f, w, 1.75f, h)), angle, 1.0f, 0.5f, "wood_tile");
    create_bed(components, sum(pos, lin_comb(-0.5f, w, 5.0f, h)), angle + 1.5f * M_PI);
    create_lamp(components, sum(pos, lin_comb(6.5f, w, 6.5f, h)));

    // Living room
    create_wall(components, sum(pos, lin_comb(0.0f, w, -1.75f, h)), angle, 4.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, lin_comb(-5.5f, w, -1.75f, h)), angle, 3.0f, 0.5f, "wood_tile");
    create_bench(components, sum(pos, lin_comb(-3.0f, w, -5.5f, h)), angle + M_PI);
    create_table(components, sum(pos, lin_comb(-5.5f, w, -5.5f, h)));
    create_lamp(components, sum(pos, lin_comb(-5.5f, w, -5.5f, h)));

    // Kitchen
    create_wall(components, sum(pos, lin_comb(5.5f, w, -1.75f, h)), angle, 3.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, lin_comb(0.0, w, -4.5f, h)), angle + 0.5f * M_PI, 5.0f, 0.5f, "wood_tile");
    create_stove(components, sum(pos, lin_comb(6.0f, w, -6.0f, h)), angle + M_PI);
    create_sink(components, sum(pos, lin_comb(6.0f, w, -3.5f, h)), angle + M_PI);

    create_waypoint(sum(pos, mult(9.0f, w)));
    create_waypoint(sum(pos, mult(5.0f, w)));
    create_waypoint(sum(pos, lin_comb(5.0f, w, 3.0f, h)));
    create_waypoint(sum(pos, mult(3.0f, w)));
    create_waypoint(sum(pos, lin_comb(3.0f, w, -3.0f, h)));
    create_waypoint(sum(pos, mult(-3.0f, w)));
    create_waypoint(sum(pos, lin_comb(-3.0f, w, -3.0f, h)));
    create_waypoint(sum(pos, mult(-5.0f, w)));
    create_waypoint(sum(pos, lin_comb(-5.0f, w, 3.0f, h)));

    int items = randi(1, 3);
    int locations[3] = {0, 1, 2};
    permute(locations, 3);
    for (int i = 0; i < items; i++) {
        switch (locations[i]) {
            case 0:
                create_item(components, sum(pos, lin_comb(-4.5f, w, 4.0f, h)), i);
                break;
            case 1:
                create_item(components, sum(pos, lin_comb(3.0f, w, 5.0f, h)), i);
                break;
            case 2:
                create_item(components, sum(pos, lin_comb(3.0f, w, -4.5f, h)), i);
                break;
        }
    }
}


void create_outhouse(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 3.0f, 3.0f, angle, "board_tile");

    create_wall(components, sum(pos, mult(-1.25f, w)), angle + 0.5 * M_PI, 2.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, mult(1.25f, h)), angle, 3.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, mult(-1.25f, h)), angle, 3.0f, 0.5f, "wood_tile");

    create_wall(components, sum(pos, mult(-0.5f, w)), angle, 1.0f, 2.0f, "outhouse");

    create_waypoint(sum(pos, lin_comb(2.5f, w, 0.0f, h)));
    create_waypoint(sum(pos, lin_comb(2.5f, w, 2.5f, h)));
    create_waypoint(sum(pos, lin_comb(2.5f, w, -2.5f, h)));
    create_waypoint(sum(pos, lin_comb(-2.5f, w, 2.5f, h)));
    create_waypoint(sum(pos, lin_comb(-2.5f, w, -2.5f, h)));

    create_door(sum(pos, mult(1.25f, w)), angle + 0.5f * M_PI);

    create_item(components, pos, 0);
}


void create_garage(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 9.0f, 6.0f, angle, "board_tile");

    create_wall(components, sum(pos, mult(-4.25f, w)), angle + 0.5 * M_PI, 6.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, mult(3.25f, h)), angle, 9.0f, 0.5f, "wood_tile");
    create_wall(components, sum(pos, mult(-3.25f, h)), angle, 9.0f, 0.5f, "wood_tile");

    create_waypoint(sum(pos, lin_comb(5.5f, w, 0.0f, h)));
    create_waypoint(sum(pos, lin_comb(5.5f, w, 4.5f, h)));
    create_waypoint(sum(pos, lin_comb(5.5f, w, -4.5f, h)));
    create_waypoint(sum(pos, lin_comb(-5.5f, w, 4.5f, h)));
    create_waypoint(sum(pos, lin_comb(-5.5f, w, -4.5f, h)));

    create_car(components, sum(pos, w), angle);
}


void create_mansion(ComponentData* components, sfVector2f pos) {
    float angle = 0.0f;

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = perp(w);

    create_floor(components, sum(pos, mult(14.0f, w)), 20.0f, 16.0f, angle, "board_tile");
    create_floor(components, sum(pos, lin_comb(-4.0, w, -16.0f, h)), 40.0f, 16.0f, angle, "board_tile");
    create_floor(components, sum(pos, lin_comb(-4.0, w, 16.0f, h)), 40.0f, 16.0f, angle, "board_tile");

    // Front walls
    create_wall(components, sum(pos, lin_comb(15.5f, w, -21.0f, h)), angle + 0.5f * M_PI, 6.0f, 1.0f, "brick_tile");
    create_glass(components, sum(pos, lin_comb(15.5f, w, -16.0f, h)), angle);
    create_wall(components, sum(pos, lin_comb(15.5f, w, -11.0f, h)), angle + 0.5f * M_PI, 6.0f, 1.0f, "brick_tile");
    create_waypoint(sum(pos, lin_comb(17.0f, w, -25.0f, h)));

    create_wall(components, sum(pos, lin_comb(15.5f, w, 11.0f, h)), angle + 0.5f * M_PI, 6.0f, 1.0f, "brick_tile");
    create_glass(components, sum(pos, lin_comb(15.5f, w, 16.0f, h)), angle);
    create_wall(components, sum(pos, lin_comb(15.5f, w, 21.0f, h)), angle + 0.5f * M_PI, 6.0f, 1.0f, "brick_tile");
    create_waypoint(sum(pos, lin_comb(17.0f, w, 25.0f, h)));

    // Alchove
    create_wall(components, sum(pos, lin_comb(23.5f, w, -5.0f, h)), angle + 0.5f * M_PI, 6.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(23.5f, w, 5.0f, h)), angle + 0.5f * M_PI, 6.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(19.0f, w, -7.5, h)), angle, 8.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(19.0f, w, 7.5, h)), angle, 8.0f, 1.0f, "brick_tile");
    create_door(sum(pos, lin_comb(23.5f, w, -1.0, h)), angle + 0.5f * M_PI);
    create_door(sum(pos, lin_comb(23.5f, w, 1.0, h)), angle + 1.5f * M_PI);
    create_waypoint(sum(pos, lin_comb(25.0f, w, 9.0f, h)));
    create_waypoint(sum(pos, lin_comb(25.0f, w, -9.0f, h)));

    // Right wall
    create_wall(components, sum(pos, lin_comb(-20.5f, w, -23.5f, h)), angle, 7.0f, 1.0f, "brick_tile");
    create_glass(components, sum(pos, lin_comb(-15.0f, w, -23.5f, h)), angle + 0.5f * M_PI);
    create_wall(components, sum(pos, lin_comb(-9.5f, w, -23.5f, h)), angle, 7.0f, 1.0f, "brick_tile");
    create_glass(components, sum(pos, lin_comb(-4.0f, w, -23.5f, h)), angle + 0.5f * M_PI);
    create_wall(components, sum(pos, lin_comb(1.5f, w, -23.5f, h)), angle, 7.0f, 1.0f, "brick_tile");
    create_glass(components, sum(pos, lin_comb(7.0f, w, -23.5f, h)), angle + 0.5f * M_PI);
    create_wall(components, sum(pos, lin_comb(12.5f, w, -23.5f, h)), angle, 7.0f, 1.0f, "brick_tile");

    // Left wall
    create_wall(components, sum(pos, lin_comb(-20.5f, w, 23.5f, h)), angle, 7.0f, 1.0f, "brick_tile");
    create_glass(components, sum(pos, lin_comb(-15.0f, w, 23.5f, h)), angle + 0.5f * M_PI);
    create_wall(components, sum(pos, lin_comb(-9.5f, w, 23.5f, h)), angle, 7.0f, 1.0f, "brick_tile");
    create_glass(components, sum(pos, lin_comb(-4.0f, w, 23.5f, h)), angle + 0.5f * M_PI);
    create_wall(components, sum(pos, lin_comb(1.5f, w, 23.5f, h)), angle, 7.0f, 1.0f, "brick_tile");
    create_glass(components, sum(pos, lin_comb(7.0f, w, 23.5f, h)), angle + 0.5f * M_PI);
    create_wall(components, sum(pos, lin_comb(12.5f, w, 23.5f, h)), angle, 7.0f, 1.0f, "brick_tile");
    
    // Back walls
    create_wall(components, sum(pos, lin_comb(-23.5f, w, -16.0f, h)), angle + 0.5f * M_PI, 16.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(-23.5f, w, 16.0f, h)), angle + 0.5f * M_PI, 16.0f, 1.0f, "brick_tile");
    create_waypoint(sum(pos, lin_comb(-25.0f, w, 25.0f, h)));
    create_waypoint(sum(pos, lin_comb(-25.0f, w, 15.0f, h)));
    create_waypoint(sum(pos, lin_comb(-25.0f, w, -15.0f, h)));
    create_waypoint(sum(pos, lin_comb(-25.0f, w, -25.0f, h)));

    // Yard
    create_wall(components, sum(pos, lin_comb(-22.0f, w, -7.5, h)), angle, 4.0f, 1.0f, "brick_tile");
    create_door(sum(pos, lin_comb(-19.0f, w, -7.5, h)), angle);
    create_wall(components, sum(pos, lin_comb(-6.5f, w, -7.5, h)), angle, 23.0f, 1.0f, "brick_tile");
    create_waypoint(sum(pos, lin_comb(-10.0f, w, 0.0f, h)));

    create_wall(components, sum(pos, lin_comb(-22.0f, w, 7.5, h)), angle, 4.0f, 1.0f, "brick_tile");
    create_door(sum(pos, lin_comb(-19.0f, w, 7.5, h)), angle);
    create_wall(components, sum(pos, lin_comb(-6.5f, w, 7.5, h)), angle, 23.0f, 1.0f, "brick_tile");

    create_wall(components, sum(pos, lin_comb(4.5f, w, -4.5f, h)), angle + 0.5f * M_PI, 5.0f, 1.0f, "brick_tile");
    create_waypoint(sum(pos, lin_comb(6.0f, w, 9.0f, h)));
    create_waypoint(sum(pos, lin_comb(6.0f, w, -9.0f, h)));
    create_glass(components, sum(pos, lin_comb(4.5, w, 0.0f, h)), angle);
    create_waypoint(sum(pos, lin_comb(15.0f, w, 0.0f, h)));
    create_wall(components, sum(pos, lin_comb(4.5f, w, 4.5f, h)), angle + 0.5f * M_PI, 5.0f, 1.0f, "brick_tile");

    create_fence(components, sum(pos, lin_comb(-23.75f, w, 0.0f, h)), angle + 0.5f * M_PI, 14.0f, 0.5f);
    create_waypoint(sum(pos, lin_comb(-25.25f, w, 0.0f, h)));
    create_waypoint(sum(pos, lin_comb(-22.25f, w, 0.0f, h)));

    create_waypoint(sum(pos, lin_comb(-5.0f, w, 15.0f, h)));
    create_waypoint(sum(pos, lin_comb(-5.0f, w, -15.0f, h)));
}


void create_school(ComponentData* components, sfVector2f pos) {
    float angle = rand_angle();

    sfVector2f w = polar_to_cartesian(1.0f, angle);
    sfVector2f h = perp(w);

    create_floor(components, pos, 32.0f, 26.0f, angle, "board_tile");

    // Outside walls
    create_wall(components, sum(pos, mult(-15.5f, w)), angle + 0.5 * M_PI, 26.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, mult(12.5f, h)), angle, 30.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, mult(-12.5f, h)), angle, 30.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(15.5f, w, 7.0f, h)), angle + 0.5 * M_PI, 12.0f, 1.0f, "brick_tile");
    create_wall(components, sum(pos, lin_comb(15.5f, w, -7.0f, h)), angle + 0.5 * M_PI, 12.0f, 1.0f, "brick_tile");

    create_waypoint(sum(pos, lin_comb(17.0f, w, 14.0f, h)));
    create_waypoint(sum(pos, lin_comb(17.0f, w, -14.0f, h)));
    create_waypoint(sum(pos, lin_comb(-17.0f, w, 14.0f, h)));
    create_waypoint(sum(pos, lin_comb(-17.0f, w, -14.0f, h)));

    create_waypoint(sum(pos, lin_comb(-17.0f, w, 0.0f, h)));
    create_waypoint(sum(pos, lin_comb(6.0f, w, 14.0f, h)));
    create_waypoint(sum(pos, lin_comb(6.0f, w, -14.0f, h)));
    create_waypoint(sum(pos, lin_comb(-6.0f, w, 14.0f, h)));
    create_waypoint(sum(pos, lin_comb(-6.0f, w, -14.0f, h)));

    create_lamp(components, sum(pos, lin_comb(14.0f, w, 11.0f, h)));
    create_lamp(components, sum(pos, lin_comb(14.0f, w, -11.0f, h)));
    create_lamp(components, sum(pos, lin_comb(-14.0f, w, 11.0f, h)));
    create_lamp(components, sum(pos, lin_comb(-14.0f, w, -11.0f, h)));

    for (int i = 0; i < 10; i++) {
        sfVector2f r = { randf(-14.0f, 14.0f), randf(-11.0f, 11.0f) };
        create_decal(sum(pos, lin_comb(r.x, w, r.y, h)), "blood_large", -1);
    }
    
    create_door(sum(pos, mult(15.5f, w)), angle + 0.5f * M_PI);

    for (int i = -2; i < 3; i++) {
        for (int j = -2; j < 3; j++) {
            sfVector2f r = sum(pos, lin_comb(4.0f * (i + 1), w, 4.0f * j, h));
            create_desk(components, r, angle + randf(-0.1f, 0.1f));
        }
    }

    create_boss(components, sum(pos, lin_comb(-10.0f, w, 5.0f, h)), angle + 1.5f * M_PI);
}
