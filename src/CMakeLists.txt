IF (WIN32)
    include(FetchContent)
    FetchContent_Declare(
            curl
            URL https://curl.se/windows/dl-8.17.0_2/curl-8.17.0_2-win64-mingw.zip
            URL_HASH SHA256=4efb12a1e87c44c46d56a27f5f81767557280307c5816a8ac3c645e951d83474
            FIND_PACKAGE_ARGS NAMES libcurl curl
    )
    include_directories(../cmake-build-debug/_deps/curl-src/include/)
    link_directories(../cmake-build-debug/_deps/curl-src/lib/)
    FetchContent_MakeAvailable(curl)
ENDIF()


set(SOURCE_FILES ./dyn.c ./map/map.c ./map/web.c ./map/json_lexer.c ./map/json_parse.c ./risk/risk.c ./simulation/simulation.c
        ./visual/visual.c
        models/geo.c
        models/road.c
        Debug/Logger.c
        Debug/Logger.h)

add_executable(wfrs main.c ${SOURCE_FILES})

target_link_libraries(wfrs curl)
target_link_libraries(wfrs m)
