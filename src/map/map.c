#include "map.h"

#include "../models/geo.h"
#include "json_parse.h"
#include "web.h"

#include <stdio.h>

bool get_road_segments(BoundBox bbox, RoadSegSlice* road_buf)
{
    String data_buf = {0};

    long response_code;
    // Continue requesting while the respose code isn't SUCCESS (200)
    do
    {
        response_code = send_overpass_request(&data_buf, "https://overpass-api.de/api/interpreter",
                                              OVERPASS_ROADS, bbox);
    } while (response_code != REQ_SUCCESS && printf("Retrying..."));

    if (!parse(data_buf.chars, road_buf))
    {
        printf("Failed parse JSON response\n");
        return false;
    }

    str_free(&data_buf);

    return true;
}

bool get_wind_velocity(GCoord coord, Vec2* wind_buf)
{
    // TODO: Get actual weather info
    *wind_buf = (Vec2){.x = 1., .y = 1.};
    return true;
}
