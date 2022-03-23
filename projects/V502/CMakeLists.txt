# For the sake of development, headers and source files aren't separated into a src and extern folder
set(V502Lib_SOURCES
        "vm/6502_ops.c"
        "vm/6502_vm.c"

        "assembler/asm_symbol.c"
        "assembler/assembler.c"
)

add_library(V502Lib STATIC ${V502Lib_SOURCES})
target_include_directories(V502Lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})