set(V502GUI_Sources
    "main.cpp"

    "dependencies/imgui/imgui.cpp"
    "dependencies/imgui/imgui_draw.cpp"
    "dependencies/imgui/imgui_widgets.cpp"
    "dependencies/imgui/imgui_tables.cpp"
    "dependencies/imgui/backends/imgui_impl_glfw.cpp"
    "dependencies/imgui/backends/imgui_impl_opengl3.cpp"
)

if (DEFINED WIN32)

else()
    set(V502GUI_Libs GL GLEW glfw dl)
endif()

add_executable(V502GUI ${V502GUI_Sources})
target_link_libraries(V502GUI V502Lib ${V502GUI_Libs})
target_include_directories(V502GUI PUBLIC ${PROJECTS_DIR} "dependencies/imgui")