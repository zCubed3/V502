# For the sake of development, headers and source files aren't separated into a src and extern folder
set(v502lib_SOURCES
        "vm/6502_ops.c"
        "vm/6502_vm.c"

        "assembler/asm_symbol.c"
        "assembler/assembler.c"
        "assembler/disassembler.c"
)

add_library(v502lib STATIC ${v502lib_SOURCES})
target_include_directories(v502lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_target_properties(v502lib PROPERTIES OUTPUT_NAME v502)