#include "Debug/Logger.h"
#include "dyn.h"
#include "map/map.h"
#include "models/fire.h"
#include "models/geo.h"
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
    pthread_create(&stopCheckThread, NULL, (void*)simulation_stop_check_thread, NULL);
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

        case SIGFPE:
            debug_log(ERROR, "(SIGFPE) PROGRAM CLOSED DUE TO A FLOATING POINT ERROR!");
            fprintf(stderr, "(SIGFPE) PROGRAM CLOSED DUE TO A FLOATING POINT ERROR!");
            exit(EXIT_FAILURE);

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

    //
    // Input handling
    //
    printf("To create a risk assessment, you will need to provide coordinates to form a square "
           "bounding box.\n");
    printf("(A bounding box is made up of two coordinates, the top left corner of the box and the "
           "bottom right)\n\n");
    printf("If you wish to test the application, enter -1000 below to use a predefined area.\n");
    printf("To start, please provide the latitude of the first coordinate: ");

    BoundBox bbox = {};
    double inputVal = 0;
    int coordsRead = 0;
    bool isTestRun = false;

    while (coordsRead < 4)
    {
        const int result = scanf("%lf", &inputVal);

        // Used for testing, sets the bounding box to around Cassiopeia
        if (inputVal == -1000)
        {
            isTestRun = true;
            bbox = (BoundBox){
                .c1 = {.lat = 57.008437507228265, .lon = 9.98708721386485},
                .c2 = { .lat = 57.01467041792688, .lon = 9.99681826817088}
            };

            printf("Using coordinates for bounding box around Cassiopeia (~11cm "
                   "precision):\ncoordinate 1: %.6f, %.6f\ncoordinate 2: %.6f, %.6f\n",
                   bbox.c1.lat, bbox.c1.lon, bbox.c2.lat, bbox.c2.lon);

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
            printf("Input error: Latitude can only be between 90 and -90.\nPlease try a different "
                   "value: ");
            continue;
        }

        // Ensure the input is valid for longitude
        if ((inputVal > 180 || inputVal < -180) && (coordsRead == 1 || coordsRead == 3))
        {
            printf("Input error: Longitude can only be between 180 and -180\nPlease try a "
                   "different value: ");
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
                printf("All coordinates have been assigned, starting risk assessment with the "
                       "following coordinates (~11cm precision):\ncoordinate 1: %.6f, "
                       "%.6f\ncoordinate 2: %.6f, %.6f\n",
                       bbox.c1.lat, bbox.c1.lon, bbox.c2.lat, bbox.c2.lon);
                break;

            default:
                break;
        }

        coordsRead++;
    }

    init_console();

    printf(MOVE_CURSOR_HOME_ANSI "\e[?25l");
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
    printf("FOUND %zu ROADS\n", roads.len);
    debug_log(MESSAGE, "FOUND %zu ROADS!", roads.len);

    size_t road_node_sum = 0;
    for (size_t i = 0; i < roads.len; i++)
        road_node_sum += roads.items[i].nodes.len;

    debug_log(MESSAGE, "\t\tWITH %zu NODES", road_node_sum);



    GCoord bbox2 = (GCoord){.lat = 38.788000, .lon = -79.182000};

    debug_log(MESSAGE, "Getting fire data...");
    printf("\033[32mGetting fire data...\033[s\n\033[0m");
    FireSlice fire_slice = {0};
    if (!isTestRun)
    {
        if (!get_fire_areas(bbox.c1, &fire_slice))
        {
            debug_log(MESSAGE, "Could not find fireArea");
            return 1;
        }
    }
    else
    {
        // Handcrafted FireArea struct, to be replaced by the return of get_fire_areas above
        FireArea fire_area = (FireArea){
            .gcoord =
                local_to_global((LCoord){.x = 1, .y = 1},
                bbox, VIEWPORT_HEIGHT, VIEWPORT_WIDTH),
            .lcoord = (LCoord){.x = 1, .y = 1},
            .temperature = 400,
            .frp = 0.41,
            .category = "WF"
        };

        FireArea fire_area_2 = (FireArea){
            .gcoord = local_to_global(
                (LCoord){.x = (double)VIEWPORT_WIDTH / 2, .y = (double)VIEWPORT_HEIGHT / 2},
                bbox,
                VIEWPORT_HEIGHT, VIEWPORT_WIDTH),
            .lcoord = (LCoord){.x = (double)VIEWPORT_WIDTH / 2, .y = (double)VIEWPORT_HEIGHT / 2},
            .temperature = 600,
            .frp = 0.75,
            .category = "WF"
        };

        FireArea area[] = {fire_area, fire_area_2};
        fire_slice = (FireSlice)slice_from(area, 2);
    }

    debug_log(MESSAGE, "FOUND %llu FIRES\n", fire_slice.len);
    printf("FOUND %llu FIRES\n", fire_slice.len);

    printf("\33[u\033[0J\033[32mSuccess!\033[0m\n");

    //
    // Temporary testing of assess_roads function
    //



    VegSlice veg_slice = {0};

    if (!get_vegetation(bbox, &veg_slice))
        return 1;

    printf("FOUND %zu AREAS!\n", veg_slice.len);
    debug_log(MESSAGE, "FOUND %zu VEGETATION AREAS!", veg_slice.len);
    size_t veg_node_sum = 0;
    for (size_t i = 0; i < veg_slice.len; i++)
        veg_node_sum += veg_slice.items[i].area.len;
    debug_log(MESSAGE, "\t\tWITH %zu NODES", veg_node_sum);

    // GCoord check_gcoord = {.lat = 57.012879168843696, .lon = 9.991665773980634};
    GCoord check_gcoord = {.lat = 57.01410957399425, .lon = 9.992155908831906};

    LCoord check_coord = global_to_local(check_gcoord, bbox, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    VegType veg_type;
    if (coord_has_vegetation(check_coord, &veg_type, veg_slice, 0.001, bbox, VIEWPORT_WIDTH,
                             VIEWPORT_HEIGHT))
    {
        debug_log(MESSAGE, "FOUND VEGTYPE OF {tag: %d} FOR { x: %lf, y: %lf }", veg_type,
                  check_coord.x, check_coord.y);
    }
    else
    {
        debug_log(MESSAGE, "COULDN'T FIND ANY VEGETATION DATA FOR { x: %lf, y: %lf }", veg_type,
                  check_coord.x, check_coord.y);
    }

    printf("FOUND VEGTYPE OF {tag: %d}\n", veg_type);
    VegType max_type = VEG_NONE;
    for (size_t i = 0; i < veg_slice.len; i++)
    {
        if (veg_slice.items[i].type >= max_type)
            max_type = veg_slice.items[i].type;
    }
    debug_log(MESSAGE, "MAX VEGETATION TYPE AFTER PARSE: %d", max_type);

    set_bounding_box(bbox);
    prepend_console_command(&stop_program, "EXIT");
    prepend_console_command(&draw_console, "REFRESH CONSOLE");
    prepend_console_command(&run_simulation, "RUN SIMULATION");
    assess_roads(&roads, &fire_slice, &veg_slice);
    draw_current_state(roads, fire_slice, veg_slice);
    while (programIsRunning)
    {
        // This is just to get the program to shut up about it "not being modified in the loop"
        programIsRunning = true;
        execute_command();
    }

    return 0;
}
