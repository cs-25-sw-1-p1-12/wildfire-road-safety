#ifndef ROAD_DATA_H
#define ROAD_DATA_H

#include "../dyn.h"
#include "fire.h"
#include "geo.h"

#include <stddef.h>

typedef struct
{
    size_t id;
    GCoord coords;
} RoadNode;

typedef SliceDef(RoadNode) NodeSlice;

typedef int RoadRisk;

typedef enum
{
    RISK_ASPHALT_MELT = 0b0001,
    RISK_NEAR_FIRE = 0b0010,
} RoadRiskReason;

typedef struct
{
    /// The ID of the road
    size_t id;
    /// The nodes that make up the road
    NodeSlice nodes;
    /// The risk factor of the road
    RoadRisk risk;
    /// The reason for the risk of the road
    RoadRiskReason risk_reason;
    /// The name of the road.
    /// The pointer is NULL if the road does not have a name specified.
    char* name;
    /// The material of the road.
    /// The pointer is NULL if the road does not have a material specified.
    char* material;
} RoadSeg;


typedef SliceDef(RoadSeg) RoadSegSlice;

float GetFireDstToRoad(RoadSeg road, FireArea fire);
void assign_roads(int number_of_roads, RoadSeg road, FireArea fire);


#endif // ROAD_DATA_H
