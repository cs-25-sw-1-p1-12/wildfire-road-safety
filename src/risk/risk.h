#ifndef RISK_H
#define RISK_H

#include "../models/fire.h"
#include "../models/road.h"
#include "../models/vegetation.h"

/// Assesses the risk of a road. Putting the risk into road.risk and returning it
RoadRisk assess_road(RoadSeg* road, FireVec* fires, VegSlice vegetation);

/// Assesses all roads in a list, putting their risk into their data
void assess_roads(RoadSegSlice* roads, FireSlice fires, VegSlice vegetation);

#endif // RISK_H
