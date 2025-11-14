# add_subdirectory(map)
# add_subdirectory(models)
# add_subdirectory(risk)
# add_subdirectory(simulation)
# add_subdirectory(visual)

set(SOURCE_FILES ./map/map.c ./risk/risk.c ./simulation/simulation.c
    ./visual/visual.c)

add_executable(wfrs main.c ${SOURCE_FILES})

target_link_libraries(wfrs curl)
