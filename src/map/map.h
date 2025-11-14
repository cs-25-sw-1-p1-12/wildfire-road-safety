#ifndef MAP_H
#define MAP_H

#include "../models/geo.h"
#include "../models/road.h"

#include <stdbool.h>

/// Get all the road segments from OpenStreetMaps API
bool get_road_segments(BoundBox bbox, RoadSegSlice* slice);

bool get_fire_areas(BoundBox bbox, FireSlice* fire_buf);

bool get_wind_velocity(GCoord coord, Vec2* wind_buf);

#endif // MAP_H
