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
    std::stringstream call_stream;

    char path_buf[256];
    memset(path_buf, 0, 256);

    bool auto_cycle = false;
    int cycle_interval = 1000; // 1 ms

    float cycle_wait = 0.0F;

    // Memory for the 16x16 image that lives in page 5000 - 52FF
    GLuint image_buffer;
    glGenTextures(1, &image_buffer);
    glBindTexture(GL_TEXTURE_2D, image_buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("CPU");

        bool manual_cycle = ImGui::Button("Step");

        ImGui::SameLine();

        ImGui::Checkbox("Auto Step", &auto_cycle);
        ImGui::InputInt("Step Interval (ms)", &cycle_interval);

        if (cycle_interval < 0)
            cycle_interval = 0;

        if (auto_cycle || manual_cycle) {
            cycle_wait += imguiIO.DeltaTime;

            if (cycle_wait >= (float)cycle_interval / 1000.0F || manual_cycle) {
                cycle_wait = 0;

                try {
                    cpu->cycle();
                } catch (std::exception& err) {
                    call_stream << "Encountered exception while trying to cycle the CPU (check console for specifics!):\n" << err.what() << "\n" << std::endl;
                }

                // We refresh the image here instead because of performance
                uint8_t pixels[256 * 3];
                for (int p = 0; p < 256; p++) {
                    auto o = p * 3;
                    pixels[o] = cpu->get_at_page(0x50, p);
                    pixels[o + 1] = cpu->get_at_page(0x51, p);
                    pixels[o + 2] = cpu->get_at_page(0x52, p);
                }

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 16, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

            }
        } else
            cycle_wait = 0.0F;

        memory_stream.str("");
        memory_stream.clear();
        memory_stream << std::hex << std::flush;

        memory_stream << "CPU Info:\n\n";

        memory_stream << "Flags:\n";
        memory_stream << "| C Z I D - B V N |\n";
        memory_stream << "| ";

        for (uint8_t t = 0; t < 8; t++) {
            memory_stream << ((cpu->flags >> t) & 1) << " ";
        }

        memory_stream << "|\n\n";
        memory_stream << "Registers: \n";

        memory_stream << "| IX = " << PAD_HEX_LO << +cpu->index_x;
        memory_stream << " | IY = " << PAD_HEX_LO << +cpu->index_y;
        memory_stream << " | AC = " << PAD_HEX_LO << +cpu->accumulator;
        memory_stream << " | ST = " << PAD_HEX_LO << +cpu->stack_ptr;
        memory_stream << " | PC = " << PAD_HEX << +cpu->program_counter;
        memory_stream << " |        \n\n";

        ImGui::Text("%s", memory_stream.str().c_str());

        ImGui::End();

        ImGui::Begin("Memory");
        ImGui::InputInt("Page", &page_number);

        if (page_number < 0x00)
            page_number = 0x00;

        if (page_number > 0xFF)
            page_number = 0xFF;

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

        ImGui::Begin("Program");

        ImGui::InputTextWithHint("Bin File", "ex: test.bin", path_buf, 256);

        if (ImGui::Button("Load Bin")) {
            std::ifstream bin_file(path_buf);

            if (bin_file.is_open()) {
                sys_memory->copy_from(bin_file);
                cpu->reset();
            } else {
                call_stream << "Failed to load binary at '" << path_buf << "', does it exist? Do you have access to it?\n" << std::endl;
            }
        }

        memory_stream.str("");
        memory_stream.clear();
        memory_stream << std::hex << std::flush;

        ImGui::Text("Program Memory Slice:");

        auto lower = (cpu->program_counter) / 16;
        auto upper = (cpu->program_counter + 16) / 16;

        memory_stream << "              ";
        for (int x = lower; x < upper; x++) {
            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                memory_stream << (idx == cpu->program_counter ? "vv" : "  ") << " ";
            }
        }
        memory_stream << "\n";

        for (int x = lower; x < upper; x++) {
            memory_stream << PAD_HEX << x * 16 << " -> ";
            memory_stream << PAD_HEX << ((x + 1) * 16) - 1 << ": ";

            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                int value = +sys_memory->at(idx);
                memory_stream << PAD_HEX_LO << value << " ";
            }
            memory_stream << "\n";
        }

        memory_stream << std::endl;
        ImGui::Text("%s", memory_stream.str().c_str());

        ImGui::End();

        ImGui::Begin("Simulation Log");

        if (ImGui::Button("Clear")) {
            call_stream.str("");
            call_stream.clear();
        }

        ImGui::Text("%s", call_stream.str().c_str());

        ImGui::End();

        ImGui::Begin("Image");

        ImGui::Text("Represents 0x5000 -> 0x52FF");
        ImGui::Text("R = 0x5000 -> 0x50FF");
        ImGui::Text("G = 0x5100 -> 0x51FF");
        ImGui::Text("B = 0x5200 -> 0x52FF");

        ImGui::Image((void*)image_buffer, ImVec2(128, 128));

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}