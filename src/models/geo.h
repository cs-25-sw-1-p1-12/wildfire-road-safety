#ifndef GEO_H
#define GEO_H

#include "../dyn.h"

#include <stddef.h>

typedef struct
{
    double lat;
    double lon;
} GCoord;

typedef struct
{
    double x;
    double y;
} LCoord;

/// A polygon, consisting of a list of global coordintes as its vertices
typedef VecDef(GCoord) GPoly;

typedef struct
{
    double x;
    double y;
} Vec2;

typedef struct
{
    GCoord c1;
    GCoord c2;
} BoundBox;

LCoord global_to_local(GCoord gcoord, BoundBox bbox, size_t height, size_t width);

/// Get a bbox around a global coordinate.
/// The global coordinate will be the center of the bbox.
/// The width and height are the total width and height of the new bbox, and is in the unit
/// kilometers.
BoundBox bbox_from_coord(GCoord gcoord, double width_km, double height_km);

void print_local(LCoord local);

#endif // GEO_H
