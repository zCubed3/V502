# For the sake of ease of development, headers and source files aren't separated into a src and extern folder
set(V502Lib_SOURCES
    "components/mos6502.cpp"
    "components/memory.cpp"

    "assembler/assembler.cpp"
)

add_library(V502Lib STATIC ${V502Lib_SOURCES})