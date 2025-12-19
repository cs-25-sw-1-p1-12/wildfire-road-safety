#ifndef VEGETATION_H
#define VEGETATION_H

#include "../dyn.h"
#include "geo.h"


/// The type of vegetation in an area
typedef enum
{
    VEG_NONE,
    VEG_ROCK,
    VEG_SAND,
    VEG_BUILDINGS,
    VEG_WATER,
    VEG_WETLAND,
    VEG_FARMLAND,
    VEG_GRASS,
    VEG_SHRUBLAND,
    VEG_FOREST,
} VegType;

typedef struct
{
    size_t id;
    GPoly area;
    VegType type;
} VegData;

typedef SliceDef(VegData) VegSlice;


bool is_coord_in_area(LCoord coord, LPoly area, double tolerance);

bool coord_has_vegetation(LCoord coord, VegType* type, VegSlice data, double tolerance,
                          BoundBox gbbox, size_t width, size_t height);

#endif // VEGETATION_H
