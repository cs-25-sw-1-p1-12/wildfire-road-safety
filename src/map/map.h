#ifndef MAP_H
#define MAP_H

#include "../models/geo.h"
#include "../models/road.h"

/// Get all the road segments from OpenStreetMaps API
RoadSegSlice get_road_segments(BoundBox bbox);

FireSlice get_fire_areas(BoundBox bbox);

Vec2 get_wind_velocity(GCoord coord);

#endif // MAP_H
