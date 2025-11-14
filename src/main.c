#include "map/map.h"

#include <stdio.h>
#include "./models/geo.h"
int main()
{
        GCoord point={50, -35};
        GCoord point1={25, -50};
        GCoord point2={75, -20};
        BoundBox bbox={point1, point2};
        size_t height=100, width=100;

        print_local(global_to_local(point, bbox, height, width));

}
