cmake_minimum_required(VERSION 4.0)
project(wildfire-road-safety C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

add_custom_target(run_external
        COMMAND open -a Terminal "${CMAKE_CURRENT_BINARY_DIR}/src/wfrs"
        DEPENDS wfrs
)