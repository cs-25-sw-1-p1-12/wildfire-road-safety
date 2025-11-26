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
            break;
        case SIGFPE:
            debug_log(ERROR, "(SIGFPE) PROGRAM CLOSED DUE TO A FLOATING POINT ERROR!");
            fprintf(stderr, "(SIGFPE) PROGRAM CLOSED DUE TO A FLOATING POINT ERROR!");
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



    //
    // Temporary testing of assess_roads function
    //
    FireSlice tempFires = slice_with_len(FireSlice, 2);

    tempFires.items[0] = (FireArea){
        .bbox = (BoundBox){.c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
                           .c2 = {.lat = 57.01467041792688, .lon = 9.99681826817088}},
        .spread_delta = 0.3,
        .temperature = 326
    };

    tempFires.items[1] = (FireArea){
        .bbox = (BoundBox){.c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
                           .c2 = {.lat = 57.01467041792688, .lon = 9.99681826817088}},
        .spread_delta = 0.7,
        .temperature = 402
    };

    assess_roads(&roads, tempFires);
    slice_free(&tempFires);
}
