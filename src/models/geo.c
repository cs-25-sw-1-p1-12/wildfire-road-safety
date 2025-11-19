#include <stdio.h>
#include "geo.h"
LCoord global_to_local(GCoord gcoord, BoundBox bbox, size_t height, size_t width);



LCoord global_to_local(GCoord gcoord, BoundBox bbox, size_t height, size_t width)
{
    // Set boarders for local coordinates.

    double horistontal = (gcoord.lon-bbox.c1.lon) / (bbox.c2.lon-bbox.c1.lon);
    // Finds the x-axis for the local grid.

    double vertical =    (bbox.c2.lat-gcoord.lat) / (bbox.c2.lat-bbox.c1.lat);
    // Finds the y-axis for the local grid.

    // Place local coordinates within the boarders.
    LCoord local;
    local.x=(size_t)(horistontal*((double)width-1));
    local.y=(size_t)(vertical*((double)height-1));
    return local;
}
void print_local(LCoord local)
{
    double x=(double)local.x;
    double y=(double)local.y;
    printf("%lf / %lf", x, y);
}

