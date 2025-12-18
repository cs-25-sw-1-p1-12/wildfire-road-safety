#include "risk.h"

#include "../models/road.h"
#include "../visual/visual.h"


#include <math.h>
#include <stdlib.h>

static const double wildfireRiskMultiplier = 2;
static const float nearbyFireThreshold = 200;


void assess_roads(RoadSegSlice* roads, FireSlice fires, VegSlice vegetation)
{
    for (int i = 0; i < roads->len; i++)
    {
        RoadSeg road = roads->items[i];

        // Get nearby fires
        FireVec nearbyFires = {0};
        for (int fi = 0; fi < fires.len; fi++)
        {
            if (GetFireDstToRoad(road, fires.items[fi]) > nearbyFireThreshold)
                continue;

            vec_push(&nearbyFires, fires.items[fi]);
        }

        assess_road(&road, &nearbyFires, vegetation);
        // printf("Road with id '%zu' rated with a risk value of %d\n", road.id, road.risk);
        debug_log(MESSAGE, "Road with id '%llu' rated with a risk value of %d", road.id, road.risk);
        vec_free(nearbyFires);
        roads->items[i] = road;
    }
}

double vegetation_risk_multiplier(VegType type)
{
    switch (type)
    {
        case VEG_ROCK:
        case VEG_SAND:
            return 0.1;

        case VEG_BUILDINGS:
            return 0.5;

        case VEG_GRASS:
            return 2;

        case VEG_FARMLAND:
            return 1.9;

        case VEG_FOREST:
            return 1.3;

        case VEG_WATER:
            return 0;

        default:
            return 1;
    }
}

RoadRisk assess_road(RoadSeg* road, FireVec* fires, VegSlice vegetation)
{
    double risk = 0;
    const double avgCarSpeed = road->speed_limit;
    const double avgFireSpeed = 6.14;

    double roadLength = GetRoadLength(*road);
    for (int i = 0; i < fires->len; i++)
    {
        double localRisk = 0;

        const FireArea fire = fires->items[i];

        const double dst = GetFireDstToRoad(*road, fire);
        if (dst == INFINITY)
        {
            debug_log(ERROR, "GetFireDstToRoad: Length is infinite!");
            assert(dst == INFINITY);
        }
        const double fireEta = dst / avgFireSpeed;
        const double carEta = roadLength / avgCarSpeed;
        const double firstToReachModifier = carEta / MAX(1, fireEta);
        // debug_log(MESSAGE, "firstToReachModifier: %lf", firstToReachModifier);
        // debug_log(MESSAGE, "FireEta: %lf, CarEta: %lf", fireEta, carEta);
        localRisk += firstToReachModifier;


        // TEMPORARY RISK CALCULATION LOGIC
        // localRisk = fire.temperature / 100.0 * fire.weatherIndex;
        // localRisk *= dst / 10.0;
        //
        if (strcmp(fire.category, "WF") == 0)
        {
            localRisk *= wildfireRiskMultiplier;
        }

        VegType veg_type = VEG_NONE;
        //Fire vegetation multiplier
        LCoord lCoord = global_to_local(fire.gcoord, globalBounds, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
        if (coord_has_vegetation(lCoord, &veg_type, vegetation, 1, globalBounds, VIEWPORT_WIDTH,
                                 VIEWPORT_HEIGHT))
        {
            localRisk *= vegetation_risk_multiplier(veg_type);
        }
        //Road vegetation multiplier
        lCoord = global_to_local(get_closest_road_node(*road, fire.gcoord)->coords, globalBounds,
                                 VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
        veg_type = VEG_NONE;
        if (coord_has_vegetation(lCoord, &veg_type, vegetation, 1, globalBounds, VIEWPORT_WIDTH,
                                 VIEWPORT_HEIGHT))
        {
            localRisk *= vegetation_risk_multiplier(veg_type);
        }
        risk += localRisk;
    }

    road->risk = (RoadRisk)ceil(risk);
    return (RoadRisk)risk;
}