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

    // Bbox for area around Cassiopeia
    BoundBox bbox = (BoundBox){
        .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
        .c2 = { .lat = 57.01467041792688, .lon = 9.99681826817088}
    };

    RoadSegSlice roads = {0};
    if (!get_road_segments(bbox, &roads))
        return 1;

    printf("FOUND %zu ROADS\n", roads.len);


<<<<<<< Updated upstream


    GCoord bbox2 = (GCoord){.lat = 38.788000, .lon = -79.182000};

    FireSlice fire_slice = {0};
=======
    printf("\033[32mGetting fire data...\033[s\n\033[0m");
    FireSlice fires = {0};
>>>>>>> Stashed changes
    // UNCOMMENT ONLY WHEN TESTING API
    // MAKE SURE TO COMMENT OUT THE LINE BELOW WHEN NOT IN USE

    if (!get_fire_areas(bbox.c1, &fires))
    {
        debug_log(ERROR, "Failed!");
        printf("\n\033[0mPress any key to exit program...");
        printf("\033[31m\033[u Failed!");

        getchar();
        close_console();
        printf("\e[?25h");
    }
    //get_fire_areas(bbox2, &fire_slice); // Rewrite JSON parser to create a FireSlice from this



<<<<<<< Updated upstream
=======
    printf("\33[u\033[0J\033[32mSuccess!\033[0m\n");
    debug_log(MESSAGE, "FOUND %zu FIRES!", fires.len);
>>>>>>> Stashed changes

    //
    // Temporary testing of assess_roads function
    //
    //FireSlice tempFires = slice_with_len(FireSlice, 2);


    // Handcrafted FireArea struct
    /*FireArea fire_area = (FireArea){
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
    tempFires.items[1] = fire_area_2;*/

<<<<<<< Updated upstream
    assess_roads(&roads, tempFires);

    slice_free(&tempFires);

=======
    assess_roads(&roads, fires);
>>>>>>> Stashed changes
    VegSlice veg_slice = {0};

    if (!get_vegetation(bbox, &veg_slice))
        return 1;

    printf("FOUND %zu AREAS!\n", veg_slice.len);

    GCoord check_coord = {.lat = 57.012879168843696, .lon = 9.991665773980634};

    VegType veg_type;
    if (!coord_has_vegetation(check_coord, &veg_type, &veg_slice, 0.001))
        return 1;

    printf("FOUND VEGTYPE OF {tag: %d}\n", veg_type);

<<<<<<< Updated upstream
=======
    set_bounding_box(bbox);
    prepend_console_command(&stop_program, "EXIT");
    prepend_console_command(&draw_console, "REFRESH CONSOLE");
    prepend_console_command(&run_simulation, "RUN SIMULATION");
    draw_current_state(roads, fires, veg_slice);
    while (programIsRunning)
    {
        // This is just to get the program to shut up about it "not being modified in the loop"
        programIsRunning = true;
        //execute_command();
    }
    slice_free(&fires);
>>>>>>> Stashed changes
    return 0;
}
