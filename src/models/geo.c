#include "geo.h"

#include <math.h>
#include <stdio.h>

LCoord global_to_local(GCoord gcoord, BoundBox bbox, size_t height, size_t width)
{
    // Set boarders for local coordinates.

    double horistontal = (gcoord.lon - bbox.c1.lon) / (bbox.c2.lon - bbox.c1.lon);
    // Finds the x-axis for the local grid.

    double vertical = (bbox.c2.lat - gcoord.lat) / (bbox.c2.lat - bbox.c1.lat);
    // Finds the y-axis for the local grid.

    // Place local coordinates within the boarders.
    LCoord local;
    local.x = (size_t)(horistontal * ((double)width - 1));
    local.y = (size_t)(vertical * ((double)height - 1));
    return local;
}
void print_local(LCoord local)
{
    double x = (double)local.x;
    double y = (double)local.y;
    printf("%lf / %lf", x, y);
}

#define EARTH_RADIUS 6378

/// Formula from https://stackoverflow.com/a/7478827/27765726
BoundBox bbox_from_coord(GCoord gcoord, double width_km, double height_km)
{
    // The distance from the point to the edges
    double dx = width_km / 2;
    double dy = height_km / 2;

    double lat1 = gcoord.lat + (-dy / EARTH_RADIUS) * (180 / M_PI);
    double lon1 = gcoord.lon + (-dx / EARTH_RADIUS) * (180 / M_PI) / cos(gcoord.lat * M_PI / 180);

    double lat2 = gcoord.lat + (dy / EARTH_RADIUS) * (180 / M_PI);
    double lon2 = gcoord.lon + (dx / EARTH_RADIUS) * (180 / M_PI) / cos(gcoord.lat * M_PI / 180);

    return (BoundBox){
        .c1 = {.lat = lat1, .lon = lon1},
        .c2 = {.lat = lat2, .lon = lon2},
    };
}
