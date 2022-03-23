set(V502Assembler_SOURCES
    "main.cpp"
)

add_executable(V502Assembler ${V502Assembler_SOURCES})
target_link_libraries(V502Assembler V502Lib)
target_include_directories(V502Assembler PUBLIC ${PROJECTS_DIR})

set_target_properties(V502Assembler PROPERTIES OUTPUT_NAME asm502)

add_custom_target(copy-asm ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECTS_DIR}/asm_files
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/asm
)

add_dependencies(V502Assembler copy-asm)