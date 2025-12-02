#ifndef VEGETATION_H
#define VEGETATION_H

#include "../dyn.h"
#include "geo.h"


/// The type of vegetation in an area
typedef enum
{
    VEG_NONE,
    VEG_RESIDENTIAL,
    VEG_FARMLAND,
    VEG_GRASS,
    VEG_FOREST,
} VegType;

typedef struct
{
    GPoly area;
    VegType type;
} VegData;

typedef SliceDef(VegData) VegSlice;

#endif // VEGETATION_H
