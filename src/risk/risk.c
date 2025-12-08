#include "risk.h"
#include "../models/road.h"
#include <stdlib.h>

static const double wildfireRiskMultiplier = 2;
static const float nearbyFireThreshold = 200;


void assess_roads(RoadSegSlice* roads, FireSlice fires)
{
    for (int i = 0; i < roads->len; i++)
    {
        RoadSeg road = roads->items[i];

        // Get nearby fires
        FireVec nearbyFires = {0};
        for (int fi = 0; fi < fires.len; fi++)
        {
            if (GetFireDstToRoad(road, fires.items[fi]) > nearbyFireThreshold) continue;

            vec_push(&nearbyFires, fires.items[fi]);
        }

        RoadRisk risk = assess_road(&road, &nearbyFires);
        printf("Road with id '%llu' rated with a risk value of %d\n", road.id, road.risk);
        debug_log(MESSAGE, "Road with id '%llu' rated with a risk value of %d", road.id, road.risk);
        vec_free(nearbyFires);
        roads->items[i] = road;
    }
}

RoadRisk assess_road(RoadSeg* road, FireVec* fires)
{
    double risk = 0;

    for (int i = 0; i < fires->len; i++)
    {
        double localRisk = 0;

        FireArea fire = fires->items[i];

        // TEMPORARY RISK CALCULATION LOGIC
        localRisk = fire.temperature / 100.0 * fire.weatherIndex;
        localRisk *= GetFireDstToRoad(*road, fire) / 10.0;

        if (strcmp(fire.category, "WF") == 0)
        {
            localRisk *= wildfireRiskMultiplier;
        }

        risk += localRisk;
    }

    road->risk = (RoadRisk) risk;
    return (RoadRisk) risk;
}