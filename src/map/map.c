#include "map.h"

#include "../caching/cache.h"
#include "../models/geo.h"
#include "fire_json_parse.h"
#include "op_json_parse.h"
#include "web.h"

#include <stdio.h>

#define REQUEST_TRIES 5

bool get_road_segments(BoundBox bbox, RoadSegSlice* road_buf)
{
    String data_buf = {0};

    // Continue requesting while the respose code isn't SUCCESS (200)
    long response_code;
    for (size_t i = REQUEST_TRIES; i > 0; i--)
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

            str_empty(&data_buf);
            if (!get_cache_data(CACHE_ROAD, &data_buf) || data_buf.len == 0)
                return false;
            else
                printf("Using cached data (This may not reflect the current situation)\n");
        }
    }
    printf("Request finished..\n");

    if (!set_cache_data(CACHE_ROAD, data_buf))
        debug_log(WARNING, "Failed to write road cache data");

    if (!road_json_parse(data_buf.chars, road_buf))
    {
        printf("Failed parse JSON response\n");
        return false;
    }

    str_free(&data_buf);

    return true;
}

bool get_fire_areas(GCoord coord, FireSlice* fire_buf)
{
    // API Endpoints
    //
    // Currently active fires:
    // "https://api.ambeedata.com/fire/latest/by-place"
    // Sample url: "https://api.ambeedata.com/fire/latest/by-place?place=Virgin, UT"
    //
    // Areas in risk of fires:
    // "https://api.ambeedata.com/fire/risk/by-place"
    // Sample url: "https://api.ambeedata.com/fire/risk/by-place?place=Leon, Mexico"
    String data_buf = {0};

    long response_code;
    for (size_t i = REQUEST_TRIES; i > 0; i--)
    {
        str_empty(&data_buf);
        response_code = send_ambee_fire_request(&data_buf, coord);

        if (response_code == REQ_SUCCESS)
            break;

        printf("Got code %ld; Retrying (%zu tries left)...\n", response_code, i - 1);

        if (i == 1)
        {
            printf("Got error response:\n%s\n", data_buf.chars);
            str_empty(&data_buf);
            if (!get_cache_data(CACHE_FIRE, &data_buf) || data_buf.len == 0)
                return false;
            else
                printf("Using cached data (This may not reflect the current situation)\n");
        }
    }
    printf("Request finished..\n");

    if (!set_cache_data(CACHE_FIRE, data_buf))
        debug_log(WARNING, "Failed to write fire cache data");

    if (!fire_json_parse(data_buf.chars, fire_buf))
    {
        printf("Failed parse JSON response\n");
        return false;
    }

    str_free(&data_buf);

    return true;
}


bool get_vegetation(BoundBox bbox, VegSlice* veg_slice)
{
    String data_buf = {0};

    long response_code;
    for (size_t i = REQUEST_TRIES; i > 0; i--)
    {
        str_empty(&data_buf);
        response_code = send_overpass_request(&data_buf, "https://overpass-api.de/api/interpreter",
                                              OVERPASS_VEGETATION, bbox);

        if (response_code == REQ_SUCCESS)
            break;

        printf("Got code %ld; Retrying (%zu tries left)...\n", response_code, i - 1);

        if (i == 1)
        {
            printf("Got error response:\n%s\n", data_buf.chars);
            str_empty(&data_buf);
            if (!get_cache_data(CACHE_VEGETATION, &data_buf) || data_buf.len == 0)
                return false;
            else
                printf("Using cached data (This may not reflect the current situation)\n");
        }
    }
    printf("Request finished..\n");

    if (!set_cache_data(CACHE_VEGETATION, data_buf))
        debug_log(WARNING, "Failed to write vegetation cache data");

    vegetation_json_parse(data_buf.chars, veg_slice);

    str_free(&data_buf);

    return true;
}

bool get_wind_velocity(GCoord coord, Vec2* wind_buf)
{
    // TODO: Get actual weather info
    *wind_buf = (Vec2){.x = 1., .y = 1.};
    return true;
}
