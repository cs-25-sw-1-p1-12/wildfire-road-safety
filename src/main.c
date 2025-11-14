#include "map/map.h"

#include <stdio.h>

int main()
{
    Vec2 wind = get_wind_velocity((GCoord){.lat = 0., .lon = 0.});
    printf("Wind: x = %lf, y = %lf\n", wind.x, wind.y);
}
