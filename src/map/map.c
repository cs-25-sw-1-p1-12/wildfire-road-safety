#include "map.h"

#include "../models/geo.h"
#include "json_parse.h"
#include "web.h"

#include <stdio.h>

bool get_road_segments(BoundBox bbox, RoadSegSlice* road_buf)
{
    String data_buf = {0};

    // Continue requesting while the respose code isn't SUCCESS (200)
    long response_code;
    for (size_t i = 10; i > 0; i--)
    {
        str_empty(&data_buf);
        response_code = send_overpass_request(&data_buf, "https://overpass-api.de/api/interpreter",
                                              OVERPASS_ROADS, bbox);

        if (response_code == REQ_SUCCESS)
            break;

        printf("Got code %ld; Retrying (%zu tries left)...\n", response_code, i - 1);

        if (i == 1)
        {
            printf("Got error response:\n%s\n", data_buf.chars);
            return false;
        }
    }
    printf("Request finished..\n");


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
