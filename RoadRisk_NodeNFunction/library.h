#include <stddef.h>

#ifndef ROADRISK_NODENFUNCTION_LIBRARY_H
#define ROADRISK_NODENFUNCTION_LIBRARY_H

typedef struct {
    float lon;
    float lat;
    int x;
    int y;
} Node;

typedef struct {
    float lat1;
    float long1;
    float lat2;
    float long2;
} BoundBox;

typedef struct {
    Node* nodes;
    size_t nodeCount;
} RoadData;

typedef struct {
    BoundBox bound_box;
    int x;
    int y;
} FireData;

float GetFireDstToRoad(RoadData road, FireData fire);

#endif // ROADRISK_NODENFUNCTION_LIBRARY_H