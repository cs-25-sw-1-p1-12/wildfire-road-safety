#include "risk.h"

#include "../models/road.h"

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

        RoadRisk risk = assess_road(&road, &nearbyFires, vegetation);
        printf("Road with id '%lu' rated with a risk value of %d\n", road.id, risk);

        vec_free(nearbyFires);
    }
}

RoadRisk assess_road(RoadSeg* road, FireVec* fires, VegSlice* vegetation)
{
    double totalImpactScore = 0;

    for (int i = 0; i < fires->len; i++)
    {
        // Get the fire area at index i
        FireArea fire = fires->items[i];

        double distanceToFire = GetFireDstToRoad(*road, fire);
        double vegetationImpactScore = calc_vegetation_impact_score(vegetation);


        // Calculate a hazard score
        double hazardScore = fire.frp * pow(-distanceToFire, decayConstant) * vegetationImpactScore;

        // Calculate an exposure score
        double exposureScore = 1 / (1 + distanceToFire);

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

double calc_vegetation_impact_score(VegSlice* vegetation) {
    // Calculate vegetation impact score
    // THIS SHOULD INCLUDE THE DISTANCE TO THE VEGETATION & SIZE OF VEGETATION AREA
    double vegetationImpactScore = 0;

    for (int vi = 0; vi < vegetation->len; vi++)
    {
        VegData veg = vegetation->items[vi];

        switch (veg.type)
        {
            case VEG_GRASS: // MEDIUM RISK
                break;
            case VEG_FARMLAND: // MEDIUM/HIGH RISK
                vegetationImpactScore += 100;
                break;
            case VEG_FOREST: // HIGH RISK
                vegetationImpactScore += 150;
                break;
            case VEG_SHRUBLAND: // HIGH RISK
                vegetationImpactScore += 180;
                break;
            case VEG_SAND: // HIGH OR LOW RISK
                vegetationImpactScore -= 50;
                break;
            case VEG_ROCK: // LOW RISK
                vegetationImpactScore -= 75;
                break;
            case VEG_WETLAND: // LOW RISK
                vegetationImpactScore -= 70;
                break;
            case VEG_WATER: // LESSENS RISK
                vegetationImpactScore -= 100;
                break;
            case VEG_BUILDINGS: // SPECIAL CASE, HAS POTENTIAL TO HARM PEOPLE
                vegetationImpactScore += 200;
                break;
            case VEG_NONE:
                break;
        }
    }

    vegetationImpactScore = vegetationImpactScore / vegetation->len / 100;
    if (vegetationImpactScore < 1 && vegetationImpactScore > 0)
        vegetationImpactScore += 1;

    return vegetationImpactScore;
}
