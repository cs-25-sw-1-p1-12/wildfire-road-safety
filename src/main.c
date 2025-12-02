#include "Debug/Logger.h"
#include "dyn.h"
#include "map/map.h"
#include "models/fire.h"
#include "models/road.h"
#include "risk/risk.h"
#include "signal.h"

#include <stdio.h>

void signal_handler(int signalNum)
{
    switch (signalNum)
    {
        case SIGSEGV:
            debug_log(ERROR, "(SIGSEGV) PROGRAM CLOSED DUE TO A SEGMENTATION FAULT!");
            fprintf(stderr, "(SIGSEGV) PROGRAM CLOSED DUE TO A SEGMENTATION FAULT!");
            exit(EXIT_FAILURE);
            break;
        case SIGFPE:
            debug_log(ERROR, "(SIGFPE) PROGRAM CLOSED DUE TO A FLOATING POINT ERROR!");
            fprintf(stderr, "(SIGFPE) PROGRAM CLOSED DUE TO A FLOATING POINT ERROR!");
            exit(EXIT_FAILURE);
            break;
        default:
            break;
    }
}


int main()
{
    signal(SIGSEGV, signal_handler);
    signal(SIGFPE, signal_handler);

    // Bbox for area around Cassiopeia
    BoundBox bbox = (BoundBox){
        .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
        .c2 = { .lat = 57.01467041792688, .lon = 9.99681826817088}
    };

    RoadSegSlice roads = {0};
    if (!get_road_segments(bbox, &roads))
        return 1;

    printf("FOUND %zu ROADS\n", roads.len);




    GCoord bbox2 = (GCoord) {
        .lat = 38.788000,
        .lon = -79.182000
    };

    // UNCOMMENT ONLY WHEN TESTING API
    // MAKE SURE TO COMMENT OUT THE LINE BELOW WHEN NOT IN USE
    get_fire_areas(bbox2, &(FireSlice) {{},0}); // Rewrite JSON parser to create a FireSlice from this


    //
    // Temporary testing of assess_roads function
    //
    FireSlice tempFires = slice_with_len(FireSlice, 2);

    // Handcrafted FireArea struct
    FireArea fire_area = (FireArea) {
        .gcoord = (GCoord) {.lat = 1, .lon = 1},
        .lcoord = (LCoord) {.x = 1, .y = 1},
        .temperature = 400,
        .weatherIndex = 0.41,
        .category = "WF"
    };

    FireArea fire_area_2 = (FireArea) {
        .gcoord = (GCoord) {.lat = 2, .lon = 2},
        .lcoord = (LCoord) {.x = 2, .y = 2},
        .temperature = 600,
        .weatherIndex = 0.75,
        .category = "WF"
    };

    tempFires.items[0] = fire_area;
    tempFires.items[1] = fire_area_2;

    assess_roads(&roads, tempFires);



    slice_free(&tempFires);
    return 0;
}
