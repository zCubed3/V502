set(asm502_SOURCES
    "main.cpp"
)

add_executable(asm502 ${asm502_SOURCES})
target_link_libraries(asm502 v502lib)
target_include_directories(asm502 PUBLIC ${PROJECTS_DIR})

set_target_properties(asm502 PROPERTIES OUTPUT_NAME asm502)

add_custom_target(copy-asm ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECTS_DIR}/frontends/asm502/asm_files
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/asm
)

add_dependencies(asm502 copy-asm)