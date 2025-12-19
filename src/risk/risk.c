#include "risk.h"

#include "../models/road.h"
#include "../visual/visual.h"

#include <math.h>
#include <stdlib.h>

static const double wildfireRiskMultiplier = 2;
static const float nearbyFireThreshold = 20000;
static const double decayConstant = 2;


void assess_roads(RoadSegSlice* roads, FireSlice* fires, VegSlice* vegetation)
{
    for (int i = 0; i < roads->len; i++)
    {
        RoadSeg road = roads->items[i];

        // Get nearby fires
        FireVec nearbyFires = {0};
        for (int fi = 0; fi < fires->len; fi++)
        {
            if (GetFireDstToRoad(road, fires->items[fi]) > nearbyFireThreshold) continue;

            vec_push(&nearbyFires, fires->items[fi]);
        }

        assess_road(&road, &nearbyFires, vegetation);
        // printf("Road with id '%zu' rated with a risk value of %d\n", road.id, road.risk);
        debug_log(MESSAGE, "Road with id '%llu' rated with a risk value of %d", road.id, road.risk);
        vec_free(nearbyFires);
        roads->items[i] = road;
    }
}

RoadRisk assess_road(RoadSeg* road, FireVec* fires, VegSlice* vegetation)
{
    double totalImpactScore = 0;
    const double avgCarSpeed = road->speed_limit;
    const double avgFireSpeed = 6.14;

    double roadLength = GetRoadLength(*road);
    for (int i = 0; i < fires->len; i++)
    {
        // Get the fire area at index i
        FireArea fire = fires->items[i];

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

        double vegetationImpactScore = calc_vegetation_impact_score(road, &fire, *vegetation);


        // Calculate a hazard score
        double hazardScore = firstToReachModifier + fire.frp * pow(-dst, decayConstant) * vegetationImpactScore;

        // Calculate an exposure score
        double exposureScore = 1 / (1 + dst);

        // Multiply hazard and exposure score
        double impactScore = hazardScore * exposureScore;

        // Add the final score to the risk value
        totalImpactScore += impactScore;
    }

    // Calculate a vulnerability weight
    double vulnerabilityWeight = 1;

    if (road->material != NULL && strcmp(road->material, "asphalt") == 0)
        vulnerabilityWeight = 0.8;
    if (road->material != NULL && strcmp(road->material, "concrete") == 0)
        vulnerabilityWeight = 0.55;

    // Multiply risk by vulnerabilityWeight
    const double risk = totalImpactScore * vulnerabilityWeight;

    road->risk = (RoadRisk) risk;
    return (RoadRisk) risk;
}

double calc_vegetation_impact_score(RoadSeg* road, FireArea* fire, VegSlice vegetation) {
    double impactScore = 0;

    VegType veg_type = VEG_NONE;

    //Fire vegetation multiplier
    LCoord lCoord = global_to_local(fire->gcoord, globalBounds, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    if (coord_has_vegetation(lCoord, &veg_type, vegetation, 1, globalBounds, VIEWPORT_WIDTH,
    VIEWPORT_HEIGHT))
    {
        impactScore += get_vegetation_risk_multiplier(veg_type);
    }


    //Road vegetation multiplier
    veg_type = VEG_NONE;

    lCoord = global_to_local(get_closest_road_node(*road, fire->gcoord)->coords, globalBounds,
    VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    if (coord_has_vegetation(lCoord, &veg_type, vegetation, 1, globalBounds, VIEWPORT_WIDTH,
    VIEWPORT_HEIGHT))
    {
        impactScore += get_vegetation_risk_multiplier(veg_type);
    }

    return impactScore;
}

double get_vegetation_risk_multiplier(VegType veg_type)
{
    switch (veg_type)
    {
        case VEG_GRASS: // MEDIUM RISK
            return 1;
        case VEG_FARMLAND: // MEDIUM/HIGH RISK
            return 1.3;
        case VEG_FOREST: // HIGH RISK
            return 1.5;
        case VEG_SHRUBLAND: // HIGH RISK
            return 1.8;
        case VEG_SAND: // HIGH OR LOW RISK
        case VEG_ROCK: // LOW RISK
            return 0.95;
        case VEG_WETLAND: // LOW RISK
            return 0.9;
        case VEG_WATER: // LESSENS RISK
            return 0.8;
        case VEG_BUILDINGS: // SPECIAL CASE, HAS POTENTIAL TO HARM PEOPLE
            return 2;
        case VEG_NONE:
            return 1;
        default:
            return 1;
    }
}
