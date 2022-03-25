cmake_minimum_required(VERSION 3.20)
project(v502)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_C_COMPILER_ID}-${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_BUILD_TYPE}/")
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

set(CMAKE_C_STANDARD 99)

set(PROJECTS_DIR "${CMAKE_SOURCE_DIR}/projects")

add_subdirectory("${PROJECTS_DIR}/v502") # Core

# TODO: Make frontends optional
if (DEFINED V502_FRONTENDS)
    set(CMAKE_CXX_STANDARD 17)

    add_subdirectory("${PROJECTS_DIR}/frontends/emu502") # Loader program
    add_subdirectory("${PROJECTS_DIR}/frontends/asm502") # Assembler program
    add_subdirectory("${PROJECTS_DIR}/frontends/dasm502") # Disassembler program

    # GUI is lowest to prevent compilation disruption
    if (DEFINED V502_FRONTEND_GUI)
        add_subdirectory("${PROJECTS_DIR}/frontends/emu502-gui")
    endif()
endif()