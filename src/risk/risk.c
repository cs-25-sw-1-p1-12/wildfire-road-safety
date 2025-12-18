#include "risk.h"

#include "../models/road.h"

#include <math.h>
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
            if (GetFireDstToRoad(road, fires.items[fi]) > nearbyFireThreshold)
                continue;

            vec_push(&nearbyFires, fires.items[fi]);
        }

        RoadRisk risk = assess_road(&road, &nearbyFires);
        printf("Road with id '%zu' rated with a risk value of %d\n", road.id, road.risk);
        // debug_log(MESSAGE, "Road with id '%llu' rated with a risk value of %d", road.id,
        // road.risk);
        vec_free(nearbyFires);
        roads->items[i] = road;
    }
}

RoadRisk assess_road(RoadSeg* road, FireVec* fires)
{
    double risk = 0;
    double avgCarSpeed = 13.9;
    double avgFireSpeed = 6.14;

    double roadLength = GetRoadLength(*road);
    for (int i = 0; i < fires->len; i++)
    {
        double localRisk = 0;

        FireArea fire = fires->items[i];

        double dst = GetFireDstToRoad(*road, fire);
        if (dst == INFINITY)
        {
            debug_log(ERROR, "GetFireDstToRoad: Length is infinite!");
            assert(dst == INFINITY);
        }
        double fireEta = dst / avgFireSpeed;
        double carEta = roadLength / avgCarSpeed;
        double firstToReachModifier = carEta / MAX(1, fireEta);
        debug_log(MESSAGE, "firstToReachModifier: %lf", firstToReachModifier);
        debug_log(MESSAGE, "FireEta: %lf, CarEta: %lf", fireEta, carEta);
        risk += firstToReachModifier;


        // TEMPORARY RISK CALCULATION LOGIC
        // localRisk = fire.temperature / 100.0 * fire.weatherIndex;
        // localRisk *= dst / 10.0;
        //
        // if (strcmp(fire.category, "WF") == 0)
        // {
        //     localRisk *= wildfireRiskMultiplier;
        // }

        // risk += localRisk;
    }

    road->risk = (RoadRisk)risk;
    return (RoadRisk)risk;
}