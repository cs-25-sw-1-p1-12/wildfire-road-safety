#include "vegetation.h"

#include "geo.h"

#include <math.h>
#include <string.h>

#define MAX(a, b) (a >= b ? a : b)
#define MIN(a, b) (a <= b ? a : b)

typedef enum
{
    DIR_NONE,
    DIR_UP,
    DIR_DOWN,
} LineDir;

BoundBox get_area_bbox(GPoly area);
LineDir check_line_intersection(GCoord n1, GCoord n2, GCoord p, double tolerance);

bool coord_has_vegetation(GCoord coord, VegType* type, VegSlice* data, double tolerance)
{
    for (size_t i = 0; i < data->len; i++)
    {
        if (is_coord_in_area(coord, data->items[i].area, tolerance))
        {
            *type = data->items[i].type;
            return true;
        }
    }

    return false;
}

bool is_coord_in_area(GCoord coord, GPoly area, double tolerance)
{
    // c1 has min values, c2 has max values
    BoundBox bbox = get_area_bbox(area);

    if ((coord.lat < bbox.c1.lat - tolerance || coord.lat > bbox.c2.lat + tolerance) ||
        (coord.lon < bbox.c1.lon - tolerance || coord.lon > bbox.c2.lon + tolerance))
        return false;

    if (area.len == 0)
        return false;

    if (area.len == 1)
    {
        GCoord point = area.items[0];
        if (coord.lat >= point.lat - tolerance || coord.lon <= point.lon + tolerance)
            return true;
        else
            return false;
    }

    GCoord new_coord = coord;
    while (new_coord.lon <= bbox.c2.lon)
    {
        for (size_t i = 0; i < area.len; i += 2)
        {
            LineDir line =
                check_line_intersection(area.items[i], area.items[i + 1], new_coord, tolerance);
            if (line == DIR_NONE)
                continue;
            else if (line == DIR_DOWN)
                return false;
            else if (line == DIR_UP)
                return true;
        }
        new_coord.lon += tolerance * 2;
    }
    return false;
}

BoundBox get_area_bbox(GPoly area)
{
    double min_lat = 0.;
    double max_lat = 0.;
    double min_lon = 0.;
    double max_lon = 0.;

    for (size_t i = 0; i < area.len; i++)
    {
        GCoord vert = area.items[i];

        if (vert.lat < min_lat || min_lat == 0.)
            min_lat = vert.lat;

        if (vert.lat > max_lat || max_lat == 0.)
            max_lat = vert.lat;

        if (vert.lon < min_lon || min_lon == 0.)
            min_lon = vert.lon;

        if (vert.lon > max_lon || max_lon == 0.)
            max_lon = vert.lon;
    }

    return (BoundBox){
        .c1 = {.lat = min_lat, .lon = min_lon},
          .c2 = {.lat = max_lat, .lon = max_lon}
    };
}

LineDir check_line_intersection(GCoord n1, GCoord n2, GCoord p, double tolerance)
{
    // Check if in bounding box
    if (p.lat > MAX(n1.lat, n2.lat) + tolerance || p.lat < MIN(n1.lat, n2.lat) - tolerance ||
        p.lon > MAX(n1.lon, n2.lon) + tolerance || p.lon < MIN(n1.lon, n2.lon) - tolerance)
        return DIR_NONE;

    // Formula from
    // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
    double numerator = fabs((n2.lon - n1.lon) * p.lat - (n2.lat - n1.lat) * p.lon +
                            n2.lat * n1.lon - n2.lon * n1.lat);

    double denominator = sqrt(pow(n2.lon - n1.lon, 2) + pow(n2.lat - n1.lat, 2));

    double dist = numerator / denominator;

    if (dist > tolerance || dist < -tolerance)
        return DIR_NONE;

    if (MIN(n1.lat, n2.lat) == n2.lat)
        return DIR_DOWN;
    else
        return DIR_UP;
}
