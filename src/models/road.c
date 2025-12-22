#include "road.h"


#include "fire.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

GCoord GCoord_to_kilometer(GCoord gCoord)
{
    // https://stackoverflow.com/a/1253545
    double latUnit = 110.574;
    double lonUnit = 111.320 * cos(gCoord.lat * (3.14159265359 / 180));

    return (GCoord){.lat = latUnit * gCoord.lat, lonUnit * gCoord.lon};
}
GCoord closest_point_on_segment(const GCoord a, const GCoord b, const GCoord p)
{
    // https://chatgpt.com/
    const double ABy = b.lat - a.lat;
    const double ABx = b.lon - a.lon;

    const double APy = p.lat - a.lat;
    const double APx = p.lon - a.lon;

    const double ab2 = ABx * ABx + ABy * ABy;
    const double ap_ab = APx * ABx + APy * ABy;

    double t = ap_ab / ab2;
    if (t < 0.0)
        t = 0.0;
    if (t > 1.0)
        t = 1.0;

    GCoord coord;
    coord.lat = a.lat + ABy * t;
    coord.lon = a.lon + ABx * t;

    return coord;
}

double haversine(GCoord c1, GCoord c2)
{
    // https://www.geeksforgeeks.org/dsa/haversine-formula-to-find-distance-between-two-points-on-a-sphere/
    double lat1 = c1.lat;
    double lat2 = c2.lat;
    double lon1 = c1.lon;
    double lon2 = c2.lon;
    // distance between latitudes
    // and longitudes
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;

    // convert to radians
    lat1 = (lat1) * M_PI / 180.0;
    lat2 = (lat2) * M_PI / 180.0;

    // apply formulae
    double a = pow(sin(dLat / 2), 2) +
               pow(sin(dLon / 2), 2) *
               cos(lat1) * cos(lat2);
    double rad = 6371;
    double c = 2 * asin(sqrt(a));
    return rad * c;
}

RoadNode* get_closest_road_node(RoadSeg road, GCoord p)
{
    double distance = INFINITY;
    RoadNode* node = NULL;
    for (int i = 0; i < road.nodes.len - 1; i++)
    {
        GCoord coord =
            closest_point_on_segment(road.nodes.items[i].coords, road.nodes.items[i + 1].coords, p);
        // debug_log(MESSAGE, "coord: (%lf, %lf)", coord.lat, coord.lon);
        double dst = haversine(p, coord);
        if (distance > dst)
        {
            distance = dst;
            node = &road.nodes.items[i];
        }
    }
    if (distance >= INFINITY)
        return NULL;
    return node;
}

double get_fire_dst_to_road(RoadSeg road, FireArea fire)
{
    double distance = INFINITY;
    for (int i = 0; i < road.nodes.len - 1; i++)
    {
        GCoord coord = closest_point_on_segment(road.nodes.items[i].coords, road.nodes.items[i + 1].coords, fire.gcoord);
        // debug_log(MESSAGE, "coord: (%lf, %lf)", coord.lat, coord.lon);
        double dst = haversine(fire.gcoord, coord); //get_point_dst(road.nodes.items[i].coords, road.nodes.items[i + 1].coords, fire.gcoord, 1);
        if (distance > dst)
            distance = dst;
    }
    if (distance >= INFINITY)
        return INFINITY;
    return distance * 1000;
}

double GetRoadLength(RoadSeg road)
{
    double distance = 0;
    for (int i = 0; i < road.nodes.len - 1; i++)
    {
        // const GCoord n1 = GCoord_to_kilometer(road.nodes.items[i].coords);
        // const GCoord n2 = GCoord_to_kilometer(road.nodes.items[i + 1].coords);
        // float dst = sqrt(pow(n2.lon - n1.lon, 2) + pow(n2.lat - n1.lat, 2));
        // debug_log(MESSAGE, "hey: %lf (%lf,%lf)(%lf,%lf)", dst, n1.lat, n1.lon, n2.lat, n2.lon);
        distance += haversine(road.nodes.items[i].coords, road.nodes.items[i + 1].coords);
    }
    return distance * 1000;
}
