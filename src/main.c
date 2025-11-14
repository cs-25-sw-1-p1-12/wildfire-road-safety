#include "map/map.h"
#include "models/road.h"
#include "risk/risk.h"

#include <stdio.h>

int main()
{
    // Bbox for area around Cassiopeia
    BoundBox bbox = (BoundBox){
        .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
        .c2 = { .lat = 57.01467041792688, .lon = 9.99681826817088}
    };

    RoadSegSlice roads = {0};
    if (!get_road_segments(bbox, &roads))
        return 1;

    printf("FOUND %zu ROADS\n", roads.len);



    //
    // Temporary testing of assess_roads function
    //
    FireSlice tempFires;
    tempFires.items = malloc(sizeof(FireArea) * 2);
    tempFires.len = 2;

    tempFires.items[0] = (FireArea) {
        .bbox = (BoundBox) {
            .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
            .c2 = { .lat = 57.01467041792688, .lon = 9.99681826817088}
        },
        .spread_delta = 0.3,
        .temperature = 326
    };

    tempFires.items[1] = (FireArea) {
        .bbox = (BoundBox) {
            .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
            .c2 = { .lat = 57.01467041792688, .lon = 9.99681826817088}
        },
        .spread_delta = 0.7,
        .temperature = 402
    };

    assess_roads(&roads, tempFires);
    Vec2 wind = get_wind_velocity((GCoord){.lat = 0., .lon = 0.});
    printf("Wind: x = %lf, y = %lf\n", wind.x, wind.y);
}
