#ifndef FIRE_DATA_H
#define FIRE_DATA_H

#include "../dyn.h"
#include "./geo.h"

typedef struct
{
    GCoord gcoord;
    LCoord lcoord;
    double temperature;
    double weatherIndex;
    char* category;
} FireArea;

typedef SliceDef(FireArea) FireSlice;
typedef VecDef(FireArea) FireVec;

#endif // FIRE_DATA_H
