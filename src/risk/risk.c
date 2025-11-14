#include "risk.h"
#include "../models/road.h"
#include <stdlib.h>

void assess_roads(RoadSegSlice* roads, FireSlice fires)
{
    const int nearbyFireThreshold = 10;

    for (int i = 0; i < roads->len; i++)
    {
        RoadSeg road = roads->items[i];

        // Get nearby fires
        FireVec nearbyFires = {0};
        for (int fi = 0; fi < fires.len; fi++)
        {
            if (fire_distance_from_road(road, fires.items[fi]) > nearbyFireThreshold) continue;

            vec_push(&nearbyFires, fires.items[fi]);
        }

        RoadRisk risk = assess_road(&road, &nearbyFires);
        printf("Road with id '%lu' rated with a risk value of %d\n", road.id, risk);

        vec_free(nearbyFires);
    }
}


int fire_distance_from_road(RoadSeg road, FireArea fire)
{
    return (int) rand() % 20;
}

RoadRisk assess_road(RoadSeg* road, FireVec* fires)
{
    double risk = 0;

    for (int i = 0; i < fires->len; i++)
    {
        double localRisk = 0;

        FireArea fire = fires->items[i];

        // TEMPORARY RISK CALCULATION LOGIC
        localRisk = (int) fire.temperature / 10.0 * fire.spread_delta;
        localRisk *= fire_distance_from_road(*road, fire) / 10.0;

        risk += localRisk;
    }

    road->risk = (RoadRisk) risk;
    return (RoadRisk) risk;
}