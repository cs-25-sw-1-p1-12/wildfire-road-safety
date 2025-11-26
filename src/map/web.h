#ifndef WEB_H
#define WEB_H

#include "../dyn.h"
#include "../models/geo.h"

#define REQ_SUCCESS 200
#define REQ_ERR_FORBIDDEN 503
#define REQ_ERR_NOT_FOUND 404

typedef enum
{
    OVERPASS_ROADS,
    OVERPASS_VEGETATION,
} OverpassData;

/// Sends a request to the given overpass url.
/// On success it will fill the output String with a json string of all the relevant in the given
/// bbox. The returned integer is the response code of the http response, which is 200 on success.
/// If the request cannot be sent, -1 will be returned.
int send_overpass_request(String* output, char* url, OverpassData data_type, BoundBox bbox);
int send_ambee_fire_request(String* output, GCoord coord, BoundBox bbox);

#endif // WEB_H
