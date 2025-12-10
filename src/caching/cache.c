#include "cache.h"

#include <stdio.h>
#include <sys/stat.h>

#define CACHE_DIR ".cache/"
#define ROADS_FILENAME "roads.json"
#define VEGETATION_FILENAME "vegetation.json"
#define FIRES_FILENAME "fires.json"

bool make_caches();

bool get_cache_data(CacheType cache_type, String* data_buf)
{
    if (!make_caches())
        return false;

    FILE* f;
    switch (cache_type)
    {
        case CACHE_ROAD:
            f = fopen(CACHE_DIR ROADS_FILENAME, "r");
            break;
        case CACHE_VEGETATION:
            f = fopen(CACHE_DIR VEGETATION_FILENAME, "r");
            break;
        case CACHE_FIRE:
            f = fopen(CACHE_DIR FIRES_FILENAME, "r");
            break;
    }

    if (f == NULL)
        return false;

    str_read_file(data_buf, f);

    fclose(f);

    return true;
}

bool set_cache_data(CacheType cache_type, String data)
{
    if (!make_caches())
        return false;

    FILE* f;
    switch (cache_type)
    {
        case CACHE_ROAD:
            f = fopen(CACHE_DIR ROADS_FILENAME, "w");
            break;
        case CACHE_VEGETATION:
            f = fopen(CACHE_DIR VEGETATION_FILENAME, "w");
            break;
        case CACHE_FIRE:
            f = fopen(CACHE_DIR FIRES_FILENAME, "w");
            break;
    }

    if (f == NULL)
        return false;

    fputs(data.chars, f);

    fclose(f);

    return true;
}

bool make_caches()
{
    struct stat stat_res;
    stat(CACHE_DIR, &stat_res);
    if (!S_ISDIR(stat_res.st_mode))
    {
        if (mkdir(CACHE_DIR, 0755) != 0)
            return false;
    }

    FILE* file;
    file = fopen(CACHE_DIR ROADS_FILENAME, "a");
    if (file == NULL)
        return false;
    fclose(file);

    file = fopen(CACHE_DIR VEGETATION_FILENAME, "a");
    if (file == NULL)
        return false;
    fclose(file);

    file = fopen(CACHE_DIR FIRES_FILENAME, "a");
    if (file == NULL)
        return false;
    fclose(file);

    return true;
}
