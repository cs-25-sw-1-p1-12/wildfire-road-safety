#include "Debug/Logger.h"
#include "map/map.h"
#include "models/road.h"
#include "risk/risk.h"
#include "visual/visual.h"

#include <stdio.h>
#include <pthread.h>
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
    while (simIsRunning)
    {
        //DO STUFF
        printf("SIM IS RUNNING!\n");
        sleep(1);
    }
    pthread_join(stopCheckThread, NULL);
    printf("SIM STOPPED!");
    sleep(1);
    draw_console();
}

void stop_program()
{
    programIsRunning = false;
}

int main()
{
    init_console();
    // Bbox for area around Cassiopeia
    BoundBox bbox = (BoundBox){
        .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
        .c2 = {.lat = 57.01467041792688, .lon = 9.99681826817088}
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
    printf("\033[0m\33[u Success!");
    debug_log(MESSAGE, "Success!");

    printf("FOUND %zu ROADS\n", roads.len);



    //
    // Temporary testing of assess_roads function
    //
    FireSlice tempFires;
    tempFires.items = malloc(sizeof(FireArea) * 2);
    tempFires.len = 2;

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

    set_bounding_box(bbox);
    prepend_console_command(&run_simulation, "RUN SIMULATION");
    prepend_console_command(&draw_console, "REFRESH CONSOLE");
    prepend_console_command(&stop_program, "EXIT");

    draw_current_state(roads, tempFires);
    while (programIsRunning)
    {
        programIsRunning = true; //This is just to get the program to shut up about it "not being modified in the loop"
        execute_command();
    }
    close_console();
}