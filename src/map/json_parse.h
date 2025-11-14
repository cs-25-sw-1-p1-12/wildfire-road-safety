#ifndef JSON_PARSE_H
#define JSON_PARSE_H

#include "../models/road.h"

#include <stdbool.h>

bool parse(char* input, RoadSegSlice* road_data);

#endif // JSON_PARSE_H
