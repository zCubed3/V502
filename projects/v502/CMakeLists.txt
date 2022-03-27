# For the sake of development, headers and source files aren't separated into a src and extern folder
set(v502lib_SOURCES
        "vm/6502_ops.c"
        "vm/6502_vm.c"

        "assembler/assembler_symbol.c"
        "assembler/assembler.c"
)

add_library(v502lib STATIC ${v502lib_SOURCES})
target_include_directories(v502lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(v502lib PUBLIC V502_INCLUDE_ASSEMBLER)

set_target_properties(v502lib PROPERTIES OUTPUT_NAME v502)

add_library(v502lib_shared SHARED "misc/shared_lib.c" ${v502lib_SOURCES})
target_include_directories(v502lib_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(v502lib_shared PUBLIC V502_SHARED_LIBRARY V502_INCLUDE_ASSEMBLER)

set_target_properties(v502lib_shared PROPERTIES OUTPUT_NAME v502)
