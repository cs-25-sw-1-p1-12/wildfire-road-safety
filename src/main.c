#include "Debug/Logger.h"
#include "dyn.h"
#include "map/map.h"
#include "models/fire.h"
#include "models/road.h"
#include "models/vegetation.h"
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



    //
    // Input handling
    //
    printf("To create a risk assessment, you will need to provide coordinates to form a square bounding box.\n");
    printf("(A bounding box is made up of two coordinates, the top left corner of the box and the bottom right)\n\n");
    printf("If you wish to test the application, enter -1000 below to use a predefined area.\n");
    printf("To start, please provide the latitude of the first coordinate: ");

    BoundBox bbox = {};
    double inputVal = 0;
    int coordsRead = 0;

    while (coordsRead < 4)
    {
        const int result = scanf("%lf", &inputVal);

        // Used for testing, sets the bounding box to around Cassiopeia
        if (inputVal == -1000)
        {
            bbox = (BoundBox){
                .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
                .c2 = { .lat = 57.01467041792688, .lon = 9.99681826817088}
            };

            printf("Using coordinates for bounding box around Cassiopeia (~11cm precision):\ncoordinate 1: %.6f, %.6f\ncoordinate 2: %.6f, %.6f\n", 
                bbox.c1.lat, 
                bbox.c1.lon, 
                bbox.c2.lat, 
                bbox.c2.lon);

            coordsRead = 4;
        }

        // Ensure the input is valid
        if (result == 0 || inputVal == 0)
        {
            printf("Input error: Invalid double value.\n");
            continue;
        }

        // Ensure the input is valid for latitude
        if ((inputVal > 90 || inputVal < -90) && (coordsRead == 0 || coordsRead == 2))
        {
            printf("Input error: Latitude can only be between 90 and -90.\nPlease try a different value: ");
            continue;
        }

        // Ensure the input is valid for longitude
        if ((inputVal > 180 || inputVal < -180) && (coordsRead == 1 || coordsRead == 3))
        {
            printf("Input error: Longitude can only be between 180 and -180\nPlease try a different value: ");
            continue;
        }

        switch (coordsRead)
        {
            case 0:
                bbox.c1.lat = inputVal;
                printf("Please provide the longitude of the first coordinate: ");
                break;

            case 1:
                bbox.c1.lon = inputVal;
                printf("Please provide the latitude of the second coordinate: ");
                break;

            case 2:
                bbox.c2.lat = inputVal;
                printf("Please provide the longitude of the second coordinate: ");
                break;

            case 3:
                bbox.c2.lon = inputVal;
                printf("All coordinates have been assigned, starting risk assessment with the following coordinates (~11cm precision):\ncoordinate 1: %.6f, %.6f\ncoordinate 2: %.6f, %.6f\n",
                    bbox.c1.lat,
                    bbox.c1.lon,
                    bbox.c2.lat,
                    bbox.c2.lon);
                break;

            default:
                break;
        }

        coordsRead++;
    }



    RoadSegSlice roads = {0};
    if (!get_road_segments(bbox, &roads))
        return 1;

    printf("FOUND %zu ROADS\n", roads.len);

    FireSlice fire_slice = {0};

    get_fire_areas(bbox.c1,
                   &fire_slice); // Rewrite JSON parser to create a FireSlice from this


    //
    // Temporary testing of assess_roads function
    //
    FireSlice tempFires = slice_with_len(FireSlice, 2);

    // Handcrafted FireArea struct, to be replaced by the return of get_fire_areas above
    FireArea fire_area = (FireArea){
        .gcoord = (GCoord){.lat = 1, .lon = 1},
        .lcoord = (LCoord){  .x = 1,   .y = 1},
        .temperature = 400,
        .weatherIndex = 0.41,
        .category = "WF"
    };

    FireArea fire_area_2 = (FireArea){
        .gcoord = (GCoord){.lat = 2, .lon = 2},
        .lcoord = (LCoord){  .x = 2,   .y = 2},
        .temperature = 600,
        .weatherIndex = 0.75,
        .category = "WF"
    };

    tempFires.items[0] = fire_area;
    tempFires.items[1] = fire_area_2;

    assess_roads(&roads, tempFires);

    slice_free(&tempFires);

    VegSlice veg_slice = {0};

    if (!get_vegetation(bbox, &veg_slice))
        return 1;

    printf("FOUND %zu AREAS!\n", veg_slice.len);

    GCoord check_coord = {.lat = 57.012879168843696, .lon = 9.991665773980634};

    VegType veg_type;
    if (!coord_has_vegetation(check_coord, &veg_type, &veg_slice, 0.001))
        return 1;

    printf("FOUND VEGTYPE OF {tag: %d}\n", veg_type);

    return 0;
}
