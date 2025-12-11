#include "Debug/Logger.h"
#include "dyn.h"
#include "map/map.h"
#include "models/fire.h"
#include "models/road.h"
#include "models/vegetation.h"
#include "risk/risk.h"
#include "signal.h"
#include "visual/visual.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

bool simIsRunning = false;
bool programIsRunning = true;
bool errorHappened = false;

void* simulation_stop_check_thread()
{
    getchar();
    simIsRunning = false;
    return 0;
}

void run_simulation()
{
    clear();
    pthread_t stopCheckThread;
    pthread_create(&stopCheckThread, NULL, simulation_stop_check_thread, NULL);
    simIsRunning = true;
    // ReSharper disable once CppDFAConstantConditions
    while (simIsRunning)
    {
        // DO STUFF
        simIsRunning = true;
        printf("SIM IS RUNNING!\n");
        sleep(1);
    }
    // ReSharper disable once CppDFAUnreachableCode
    pthread_join(stopCheckThread, NULL);
    printf("SIM STOPPED!");
    sleep(1);
    draw_console();
}

void stop_program()
{
    programIsRunning = false;
}

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
            debug_log(ERROR, "COULD NOT RECOGNISE SIGNAL");
            fprintf(stderr, "COULD NOT RECOGNISE SIGNAL");
            break;
    }
}


int main()
{
    signal(SIGSEGV, signal_handler);
    signal(SIGFPE, signal_handler);
    init_console();

    // Bbox for area around Cassiopeia
    BoundBox bbox = (BoundBox){
        .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
        .c2 = { .lat = 57.01467041792688, .lon = 9.99681826817088}
    };

    printf("\e[?25l");
    printf("\033[32mGetting road data...\033[s\n\033[0m");
    debug_log(MESSAGE, "Getting road data...");
    RoadSegSlice roads = {0};
    if (!get_road_segments(bbox, &roads))
    {
        debug_log(ERROR, "Failed!");
        printf("\n\033[0mPress any key to exit program...");
        printf("\033[31m\033[u Failed!");

        getchar();
        close_console();
        printf("\e[?25h");
        return 0;
    }
    printf("\33[u\033[0J\033[32mSuccess!\033[0m\n");
    debug_log(MESSAGE, "Success!");

    debug_log(MESSAGE, "FOUND %zu ROADS!", roads.len);

    size_t road_node_sum = 0;
    for (size_t i = 0; i < roads.len; i++)
        road_node_sum += roads.items[i].nodes.len;

    debug_log(MESSAGE, "\t\tWITH %zu NODES", road_node_sum);



    GCoord bbox2 = (GCoord){.lat = 38.788000, .lon = -79.182000};


    printf("\033[32mGetting fire data...\033[s\n\033[0m");
    FireSlice fire_slice = {0};
    // UNCOMMENT ONLY WHEN TESTING API
    // MAKE SURE TO COMMENT OUT THE LINE BELOW WHEN NOT IN USE
    get_fire_areas(bbox2,
                   &fire_slice); // Rewrite JSON parser to create a FireSlice from this

    printf("\33[u\033[0J\033[32mSuccess!\033[0m\n");

    //
    // Temporary testing of assess_roads function
    //
    FireSlice tempFires = slice_with_len(FireSlice, 2);

    // Handcrafted FireArea struct
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
    VegSlice veg_slice = {0};

    if (!get_vegetation(bbox, &veg_slice))
        return 1;

    debug_log(MESSAGE, "FOUND %zu VEGETATION AREAS!", veg_slice.len);
    size_t veg_node_sum = 0;
    for (size_t i = 0; i < veg_slice.len; i++)
        veg_node_sum += veg_slice.items[i].area.len;

    debug_log(MESSAGE, "\t\tWITH %zu NODES", veg_node_sum);

    VegType max_type = VEG_NONE;
    for (size_t i = 0; i < veg_slice.len; i++)
    {
        if (veg_slice.items[i].type >= max_type)
            max_type = veg_slice.items[i].type;
    }
    debug_log(MESSAGE, "MAX VEGETATION TYPE AFTER PARSE: %d", max_type);
    printf("DRAWING IMAGE\n");
    debug_log(MESSAGE, "DRAWING IMAGE");
    save_state_to_image("img_out.png", 512, roads, fire_slice, veg_slice);
    debug_log(MESSAGE, "FINISHED IMAGE");

    set_bounding_box(bbox);
    prepend_console_command(&stop_program, "EXIT");
    prepend_console_command(&draw_console, "REFRESH CONSOLE");
    prepend_console_command(&run_simulation, "RUN SIMULATION");
    draw_current_state(roads, tempFires, veg_slice);
    while (programIsRunning)
    {
        // This is just to get the program to shut up about it "not being modified in the loop"
        programIsRunning = true;
        execute_command();
    }
    slice_free(&tempFires);
    return 0;
}
