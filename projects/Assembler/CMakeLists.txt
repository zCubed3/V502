set(V502Assembler_SOURCES
    "main.cpp"
)

add_executable(V502Assembler ${V502Assembler_SOURCES})
target_link_libraries(V502Assembler V502Lib)
target_include_directories(V502Assembler PUBLIC ${PROJECTS_DIR})