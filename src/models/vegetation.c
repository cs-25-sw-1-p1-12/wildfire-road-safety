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


BoundBox get_area_bbox(LPoly area);
LineDir check_line_intersection(LCoord n1, LCoord n2, LCoord p, double tolerance);

bool coord_has_vegetation(LCoord coord, VegType* type, VegSlice data, double tolerance,
                          BoundBox gbbox, size_t width, size_t height)
{
    for (size_t i = 0; i < data.len; i++)
    {
        LPoly larea = vec_with_capacity(LPoly, data.items[i].area.len);

        for (size_t j = 0; j < data.items[i].area.len; j++)
        {
            GCoord vert_gcoord = data.items[i].area.items[j];
            LCoord vert_lcoord = global_to_local(vert_gcoord, gbbox, width, height);
            vec_push(&larea, vert_lcoord);
        }

        if (is_coord_in_area(coord, larea, tolerance))
        {
            *type = data.items[i].type;

            vec_free(larea);
            return true;
        }
        vec_free(larea);
    }

    *type = VEG_NONE;
    return false;
}

typedef struct
{
    LCoord n1;
    LCoord n2;
} Line;

typedef VecDef(Line) LineVec;

bool is_coord_in_area(LCoord coord, LPoly area, double tolerance)
{
    // c1 has min values, c2 has max values
    BoundBox bbox = get_area_bbox(area);

    if ((coord.y < bbox.c1.lat - tolerance || coord.y > bbox.c2.lat + tolerance) ||
        (coord.x < bbox.c1.lon - tolerance || coord.x > bbox.c2.lon + tolerance))
        return false;

    if (area.len == 0)
        return false;

    if (area.len == 1)
    {
        LCoord point = area.items[0];
        if (coord.y >= point.y - tolerance || coord.x <= point.x + tolerance)
            return true;

        return false;
    }

    LineVec lines = {0};
    size_t dupe_count = 0;

    size_t hit_count = 0;
    LCoord new_coord = coord;

    while (new_coord.x <= bbox.c2.lon + tolerance * 2)
    {
        for (size_t i = 0; i < area.len; i++)
        {
            if (i >= area.len - 1)
                break;

            LineDir line_dir =
                check_line_intersection(area.items[i], area.items[i + 1], new_coord, tolerance);
            switch (line_dir)
            {
                case DIR_DOWN:
                    return false;
                case DIR_UP:
                    // bool is_dupe = false;
                    // Line line = {.n1 = area.items[i], .n2 = area.items[i + 1]};
                    // for (size_t i = 0; i < lines.len; i++)
                    // {
                    //     Line l = lines.items[i];
                    //     if ((l.n1.x == line.n1.x && l.n1.y == line.n1.y) &&
                    //         (l.n2.x == line.n2.x && l.n2.y == line.n2.y))
                    //     {
                    //         dupe_count += 1;
                    //         is_dupe = true;
                    //     }
                    // }
                    // if (!is_dupe)
                    // {
                    //     hit_count += 1;
                    //     vec_push(&lines, line);
                    // }
                    // break;
                    return true;
                case DIR_NONE:
                    break;
            }
        }
        new_coord.x += tolerance / 5;
    }
    debug_log(MESSAGE, "DEBUG: Found %zu duplicate line hits!", dupe_count);
    vec_free(lines);
    return hit_count % 2 != 0;
    // return false
}

BoundBox get_area_bbox(LPoly area)
{
    double min_y = 0.;
    double max_y = 0.;
    double min_x = 0.;
    double max_x = 0.;

    for (size_t i = 0; i < area.len; i++)
    {
        LCoord vert = area.items[i];

        if (vert.y < min_y || min_y == 0.)
            min_y = vert.y;

        if (vert.y > max_y || max_y == 0.)
            max_y = vert.y;

        if (vert.x < min_x || min_x == 0.)
            min_x = vert.x;

        if (vert.x > max_x || max_x == 0.)
            max_x = vert.x;
    }

    return (BoundBox){
        .c1 = {.lat = min_y, .lon = min_x},
          .c2 = {.lat = max_y, .lon = max_x}
    };
}

LineDir check_line_intersection(LCoord n1, LCoord n2, LCoord p, double tolerance)
{
    // Check if in bounding box
    if (p.y > MAX(n1.y, n2.y) + tolerance || p.y < MIN(n1.y, n2.y) - tolerance ||
        p.x > MAX(n1.x, n2.x) + tolerance || p.x < MIN(n1.x, n2.x) - tolerance)
        return DIR_NONE;

    // Formula from
    // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
    double numerator = fabs((n2.x - n1.x) * p.y - (n2.y - n1.y) * p.x + n2.y * n1.x - n2.x * n1.y);

    double denominator = sqrt(pow(n2.x - n1.x, 2) + pow(n2.y - n1.y, 2));

    double dist = numerator / denominator;

    if (dist > tolerance || dist < -tolerance)
        return DIR_NONE;
    // if ((n2.x - n1.x) * (n2.y + n2.y) >= 0)
    else if (MIN(n1.y, n2.y) == n2.y)
        return DIR_UP;
    else
        return DIR_DOWN;
}
