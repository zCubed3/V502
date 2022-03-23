set(V502Loader_SOURCES
    "main.cpp"
)

add_executable(V502Loader ${V502Loader_SOURCES})
target_link_libraries(V502Loader V502Lib)
target_include_directories(V502Loader PUBLIC ${PROJECTS_DIR})

set_target_properties(V502Loader PROPERTIES OUTPUT_NAME emu502)