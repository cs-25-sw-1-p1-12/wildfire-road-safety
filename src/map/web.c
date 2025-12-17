#include "web.h"

#include <curl/curl.h>

size_t str_write(void* ptr, size_t size, size_t nmemb, String* s);
char* create_query(OverpassData data_type, BoundBox bbox);

int send_overpass_request(String* output, char* url, OverpassData data_type, BoundBox bbox)
{
    CURL* curl_handle;

    curl_handle = curl_easy_init();

    if (!curl_handle)
        return -1;

    char* query = create_query(data_type, bbox);

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_POST, 1l);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, query);

    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, str_write);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, output);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);

    CURLcode res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK)
    {
        const char* err_msg = curl_easy_strerror(res);
        str_empty(output);
        str_append(output, err_msg);
        return -1;
    }

    long ret_code;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &ret_code);

    free(query);
    if (curl_handle)
        curl_easy_cleanup(curl_handle);

    return ret_code;
}

int send_ambee_fire_request(String* output, GCoord coord)
{
    CURL* curl_handle;
    CURLcode res;

    curl_handle = curl_easy_init();

    if (!curl_handle)
        return -1;

    char builtUrl[256];

    snprintf(builtUrl, sizeof(builtUrl),
             "https://api.ambeedata.com/fire/latest/by-lat-lng?lat=%.15g&lng=%.15g", coord.lat,
             coord.lon);

    curl_easy_setopt(curl_handle, CURLOPT_URL, builtUrl);

    // Headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(
        headers, "x-api-key: 135f6b1fc2b545586ec79db94580c59874ca7bf7081c87e98370dc3190c33ac7");
    headers = curl_slist_append(headers, "Content-type: application/json");

    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);


    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, str_write);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, output);

    res = curl_easy_perform(curl_handle);
    if (res != CURLE_OK)
    {
        fprintf(stderr, "Request failed: %s\n", curl_easy_strerror(res));
    }

    long ret_code;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &ret_code);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_handle);

    return ret_code;
}


size_t str_write(void* ptr, size_t size, size_t nmemb, String* str)
{
    size_t new_len = str->len + size * nmemb;

    str_ensure_cap(str, new_len);

    memcpy(str->chars + str->len, ptr, size * nmemb);

    str->chars[new_len] = '\0';
    str->len = new_len;

    return size * nmemb;
}

#define ROAD_QUERY_FORMAT                                                                          \
    "[out:json];way[highway][\"highway\"!~\"(pedestrian|path|cycleway|footway|street_lamp|steps)"  \
    "\"][\"service\"!~\"(parking_aisle|driveway)\"][\"access\"!~\"private\"](%lf,%lf,%lf,%lf);(._" \
    ";>;);out;"

#define VEGETATION_QUERY_FORMAT                                                              \
    "[bbox:%lf,%lf,%lf,%lf][out:json];(nw[natural];nw[landuse];nw[leisure];nw[wetland];);(." \
    "_;>;);out;"

char* create_query(OverpassData data_type, BoundBox bbox)
{
    String str = {0};

    switch (data_type)
    {
        case OVERPASS_ROADS:
            str_appendf(&str, ROAD_QUERY_FORMAT, bbox.c1.lat, bbox.c1.lon, bbox.c2.lat,
                        bbox.c2.lon);
            break;
        case OVERPASS_VEGETATION:
            str_appendf(&str, VEGETATION_QUERY_FORMAT, bbox.c1.lat, bbox.c1.lon, bbox.c2.lat,
                        bbox.c2.lon);
            break;
    }

    char* chars = str_clone_chars(str);
    str_free(&str);

    return chars;
}
