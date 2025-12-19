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
    /// The speed limit of the raod in m/s. If the road doesn't have a specified speed limit, this
    /// value is set to 0.
    double speed_limit;
    /// The name of the road.
    /// The pointer is NULL if the road does not have a name specified.
    char* name;
    /// The material of the road.
    /// The pointer is NULL if the road does not have a material specified.
    char* material;
} RoadSeg;


typedef SliceDef(RoadSeg) RoadSegSlice;

double GetFireDstToRoad(RoadSeg road, FireArea fire);
double GetRoadLength(RoadSeg road);
void assign_roads(int number_of_roads, RoadSeg road, FireArea fire);
GCoord GCoord_to_kilometer(GCoord gCoord);
double haversine(GCoord c1, GCoord c2);
GCoord closest_point_on_segment(GCoord a, GCoord b, GCoord p);
RoadNode* get_closest_road_node(RoadSeg road, GCoord p);

#endif // ROAD_DATA_H
