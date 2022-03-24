set(emu502_SOURCES
    "main.cpp"
)

add_executable(emu502 ${emu502_SOURCES})
target_link_libraries(emu502 v502lib)
target_include_directories(emu502 PUBLIC ${PROJECTS_DIR})

set_target_properties(emu502 PROPERTIES OUTPUT_NAME emu502)