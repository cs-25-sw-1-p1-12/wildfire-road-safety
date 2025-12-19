#ifndef RISK_H
#define RISK_H

#include "../models/fire.h"
#include "../models/road.h"
#include "../models/vegetation.h"

/// Assesses the risk of a road. Putting the risk into road.risk and returning it
RoadRisk assess_road(RoadSeg* road, FireVec* fire, VegSlice* vegetation);

/// Assesses all roads in a list, putting their risk into their data
void assess_roads(RoadSegSlice* roads, FireSlice* fires, VegSlice* vegetation);

// Calculates the impact score of the vegetation surrounding a road
double calc_vegetation_impact_score(RoadSeg* road, FireArea* fire, VegSlice vegetation);

// Get the risk multiplier for a given vegetation type
double get_vegetation_risk_multiplier(VegType veg_type);

#endif // RISK_H
