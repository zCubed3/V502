#include <iostream>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip> // for setw and setfill

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#include <components/memory.hpp>
#include <components/mos6502.hpp>

#define PAD_HEX_LO std::setfill('0') << std::setw(2)
#define PAD_HEX std::setfill('0') << std::setw(4)

int main(int argc, char** argv) {
    if (!glfwInit()) {
        throw std::runtime_error("GLFW failed to initialize!");
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow *window = glfwCreateWindow(1280, 720, "V502 GUI", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("GLEW failed to initialize!");
    }

    V502::MOS6502 *cpu = new V502::MOS6502();

    V502::Memory *sys_memory = new V502::Memory(65536); // 64kb of memory
    cpu->system_memory = sys_memory;

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);

    ImGuiIO &imguiIO = ImGui::GetIO();
    imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    int page_number = 0;
    std::stringstream memory_stream;
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("CPU");

        ImGui::End();

        ImGui::Begin("Memory");
        ImGui::DragInt("Page", &page_number, 1.0F, 0x00, 0xFF);

        memory_stream.str("");
        memory_stream.clear();
        memory_stream << std::hex << std::flush;

        for (int x = 0; x < 16; x++) {
            memory_stream << PAD_HEX_LO << page_number << PAD_HEX_LO << x * 16 << " -> ";
            memory_stream << PAD_HEX_LO << page_number << PAD_HEX_LO << ((x + 1) * 16) - 1 << ": ";

            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                int value = +cpu->get_at_page(page_number, idx);
                memory_stream << PAD_HEX_LO << value << " ";
            }

            memory_stream << "\n";
        }
        memory_stream << std::endl;
        ImGui::Text("%s", memory_stream.str().c_str());

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}