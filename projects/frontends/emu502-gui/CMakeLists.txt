set(emu502-gui_Sources
    "main.cpp"

    "dependencies/GLAD/src/glad.c"

    "dependencies/imgui/imgui.cpp"
    "dependencies/imgui/imgui_draw.cpp"
    "dependencies/imgui/imgui_widgets.cpp"
    "dependencies/imgui/imgui_tables.cpp"
    "dependencies/imgui/imgui_demo.cpp"
    "dependencies/imgui/backends/imgui_impl_sdl.cpp"
    "dependencies/imgui/backends/imgui_impl_opengl2.cpp"
)

if (DEFINED WIN32)
    set(emu502-gui_Libs opengl32 "SDL2" "SDL2main")
else()
    set(emu502-gui_Libs GL "SDL2" "SDL2main" dl)
    set(emu502-gui_IncludeDirs "/usr/include/SDL2")
endif()

add_executable(emu502-gui ${emu502-gui_Sources})
target_link_libraries(emu502-gui ${emu502-gui_Libs})
target_include_directories(emu502-gui PUBLIC ${PROJECTS_DIR} "dependencies/imgui" "dependencies/GLAD/include" "include" ${emu502-gui_IncludeDirs})

set_target_properties(emu502-gui PROPERTIES OUTPUT_NAME emu502-gui)

if (DEFINED WIN32)
    target_link_directories(emu502-gui PUBLIC "dependencies")
endif()