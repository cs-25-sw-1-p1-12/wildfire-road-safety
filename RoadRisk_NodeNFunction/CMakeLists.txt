cmake_minimum_required(VERSION 4.0)
project(RoadRisk_NodeNFunction C)

set(CMAKE_C_STANDARD 11)

add_library(RoadRisk_NodeNFunction STATIC library.c)
