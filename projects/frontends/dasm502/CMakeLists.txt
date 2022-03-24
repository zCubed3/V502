set(dasm502_SOURCES
    "main.cpp"
)

add_executable(dasm502 ${dasm502_SOURCES})
target_link_libraries(dasm502 v502lib)
target_include_directories(dasm502 PUBLIC ${PROJECTS_DIR})

set_target_properties(dasm502 PROPERTIES OUTPUT_NAME dasm502)