#ifndef VEGETATION_H
#define VEGETATION_H

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

#endif // VEGETATION_H
