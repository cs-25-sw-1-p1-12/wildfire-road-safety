#ifndef JSON_PARSE_H
#define JSON_PARSE_H

#include "../dyn.h"
#include "../models/road.h"
#include "../models/vegetation.h"

#include <stdbool.h>

bool road_json_parse(char* input, RoadSegSlice* road_data);
bool vegetation_json_parse(char* input, VegSlice* veg_data);

#endif // JSON_PARSE_H
