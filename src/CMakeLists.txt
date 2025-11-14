set(SOURCE_FILES ./dyn.c ./map/map.c ./map/web.c ./map/json_lexer.c ./map/json_parse.c ./risk/risk.c ./simulation/simulation.c
    ./visual/visual.c)

add_executable(wfrs main.c ${SOURCE_FILES})

target_link_libraries(wfrs curl)
target_link_libraries(wfrs m)
