#ifndef CACHE_H
#define CACHE_H

#include "../dyn.h"

#include <stdbool.h>

typedef enum
{
    CACHE_ROAD,
    CACHE_VEGETATION,
    CACHE_FIRE,
} CacheType;

/// Get cached data of a certain type
bool get_cache_data(CacheType cache_type, String* data_buf);

/// Set data for caching
bool set_cache_data(CacheType cache_type, String data);

#endif // CACHE_H
