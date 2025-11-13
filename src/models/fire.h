#ifndef FIRE_DATA_H
#define FIRE_DATA_H

#include "../dyn.h"
#include "./geo.h"

typedef struct
{
    BoundBox bbox;
    LCoord lcoord;
    double spread_delta;
    double temperature;
} FireArea;

typedef SliceDef(FireArea) FireSlice;

#endif // FIRE_DATA_H
