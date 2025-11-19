#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "road.h"
#include "fire.h"

void assign_roads(int number_of_roads, RoadSeg road, FireArea fire);

float GetFireDstToRoad(RoadSeg road, FireArea fire)
{
    double distance;
    for (int i; i<road.nodes.len; i++)
    {
        double fireX=fire.lcoord.x;
        double fireY=fire.lcoord.y;

        double roadSegStartX=road.nodes.items[i].coords.lon;
        double roadSegStartY=road.nodes.items[i].coords.lat;

        double roadSegEndX=road.nodes.items[i+1].coords.lon;
        double roadSegEndY=road.nodes.items[i+1].coords.lat;

        /*calculate the distance between a point(a point of fire),
         *and a line that passes through two points (roadsegment between two nodes.
         *distnum="nummerator" in the division */
        double distnum;
        distnum = (roadSegEndY - roadSegStartY) * fireX - (roadSegEndX - roadSegStartX) * fireY +
                  (roadSegEndX * roadSegStartY) - (roadSegEndY * roadSegStartX);
        if (distnum<0)
        {
            distnum=distnum*-1;
        }

        /*distdeno="denomenator" in the division*/
        double distdeno;
        distdeno = sqrt(pow(roadSegEndY-roadSegStartY, 2)+(pow(roadSegStartY-roadSegStartX, 2)));

        distance = distnum/distdeno;
        //roadsegmentdist[i]=distance;

    } return distance;
}
void assign_roads(int number_of_roads, RoadSeg road, FireArea fire)
{
    int* roadsegments = (int *)malloc(sizeof(int) * number_of_roads);
    for (int i; i<number_of_roads; i++)
    {
        roadsegments[i]=GetFireDstToRoad(road, fire);
    }
}

