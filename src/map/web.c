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

    CURLcode res = curl_easy_perform(curl_handle);

    if (res != CURLE_OK)
        return -1;

    long ret_code;
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &ret_code);

    free(query);
    if (curl_handle)
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

#define ROAD_QUERY_FORMAT "[out:json];(way[highway](%lf,%lf,%lf,%lf););(._;>;);out;"

#define VEGETATION_QUERY_FORMAT                                                                    \
    "[bbox:(%lf,%lf,%lf,%lf)][out:json];(nwr[natural];nwr[landuse];nwr[leisure];nwr[wetland];);(." \
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
