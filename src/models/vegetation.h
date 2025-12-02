#ifndef VEGETATION_H
#define VEGETATION_H

#include "../dyn.h"
#include "geo.h"

/// The type of vegetation in an area
typedef enum
{
    VEG_GRASS,
    VEG_FOREST,
    VEG_NONE,
} VegType;

typedef struct
{
    GPoly area;
    VegType type;
} VegData;

typedef SliceDef(VegData) VegSlice;

#endif // VEGETATION_H
