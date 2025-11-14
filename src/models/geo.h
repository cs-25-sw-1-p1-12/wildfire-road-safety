#ifndef GEO_H
#define GEO_H

#include <stddef.h>

typedef struct
{
    double lat;
    double lon;
} GCoord;

typedef struct
{
    size_t x;
    size_t y;
} LCoord;

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

void print_local(LCoord local);

#endif // GEO_H
