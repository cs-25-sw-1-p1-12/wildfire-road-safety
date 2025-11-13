#ifndef SIMULATION_H
#define SIMULATION_H

#include "../models/fire.h"
#include "../models/road.h"

typedef struct
{
    RoadSegSlice roads;
    FireSlice fires;
} SimulationData;

void run_simulation_step(SimulationData* sim_data);

#endif // SIMULATION_H
