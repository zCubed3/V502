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

#define V502_INCLUDE_ASSEMBLER
#define V502_SHARED_LIBRARY
#include <v502/v502.h>

#include "math.h"

#ifdef __linux__
#define UNIX_LIKE
#include <dlfcn.h>
#endif

#define PAD_HEX_LO std::setfill('0') << std::setw(2)
#define PAD_HEX std::setfill('0') << std::setw(4)

struct DisassemblyLine {
    uint32_t location = 0;
    std::string dasm;
};

typedef v502_function_table_t*(*fptr_get_function_table)();

struct V502Library {
#ifdef UNIX_LIKE
    void* handle;
#endif

#ifdef WIN32
    HMODULE module;
#endif

    void LoadLibrary() {
#ifdef UNIX_LIKE
        if (handle) {
            dlclose(handle);
            handle = nullptr;
        }

        handle = dlopen("./lib/libv502.so", RTLD_LAZY);
#endif

#ifdef WIN32
        if (module) {
            // TODO: WIN32 UnloadLibrary;
            module = nullptr;
        }

        module = LoadLibraryA("./lib/libv502.dll");
#endif
    }

    v502_function_table_t* GetTable() {
#ifdef UNIX_LIKE
        auto fptr = (fptr_get_function_table)dlsym(handle, "v502_get_function_table");
        return fptr();
#endif
    }
};

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

    V502Library lib {};
    lib.LoadLibrary();

    auto v502_functions = lib.GetTable();

    v502_6502vm_createinfo_t createinfo {};
    createinfo.hunk_size = 0xFFFF + 1;

    v502_6502vm_t *vm = v502_functions->v502_create_vm(&createinfo);

    v502_assembler_instance_t* assembler_instance = v502_functions->v502_create_assembler();

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);

    ImGuiIO &imguiIO = ImGui::GetIO();
    imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imguiIO.Fonts->AddFontFromFileTTF("font.ttf", 14);

    int page_number = 0;
    std::stringstream memory_stream;
    std::stringstream call_stream;

    std::vector<DisassemblyLine> dasm_lines;

    char path_buf[256];
    memset(path_buf, 0, 256);

    bool auto_cycle = false;
    int cycle_interval = 1000; // 1 ms

    float cycle_wait = 0.0F;

    int substeps = 1;

    // Memory for the 16x16 image that lives in page 5000 - 52FF by default, but can be shifted
    int image_page = 0x50;
    GLuint image_buffer;
    glGenTextures(1, &image_buffer);
    glBindTexture(GL_TEXTURE_2D, image_buffer);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    bool vsync = true;
    bool dasm_dirty = false;
    bool lib_reload = false;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::Begin("V502 VM", nullptr, ImGuiWindowFlags_MenuBar);

        if (lib_reload) {
            lib.LoadLibrary();
            v502_functions = lib.GetTable();

            auto old_vm = vm;
            vm = v502_functions->v502_create_vm(&createinfo);

            auto vm_hunk = vm->hunk;
            auto vm_funcs = vm->opfuncs;

            memcpy(vm, old_vm, sizeof(v502_6502vm_t));

            vm->hunk = vm_hunk;
            memcpy(vm->hunk, old_vm->hunk, vm->hunk_length);

            vm->opfuncs = vm_funcs;

            free(old_vm->hunk);
            free(old_vm);

            free(assembler_instance);
            assembler_instance = v502_functions->v502_create_assembler();
            dasm_dirty = true;

            lib_reload = false;
        }

        if (dasm_dirty) {
            v502_binary_file_t bin {};

            bin.bytes = (char*)vm->hunk;
            bin.length = vm->hunk_length;

            v502_disassembly_options_t ops;

            ops.produce_comment = 0;
            ops.produce_memory_markers = 1;
            ops.produce_origin = 0;

            std::string raw_dasm = v502_functions->v502_disassemble_binary(assembler_instance, &bin, &ops);
            std::stringstream raw_dasm_stream(raw_dasm);

            dasm_lines.clear();

            std::string raw_dasm_line;
            while (std::getline(raw_dasm_stream, raw_dasm_line)) {
                DisassemblyLine line;

                // Trim the memory marker out and use it as a location marker
                auto comment_idx = raw_dasm_line.find_first_of(';');
                auto mem_line = raw_dasm_line.substr(0, comment_idx);

                line.location = strtol(mem_line.c_str(), nullptr, 16);
                line.dasm = raw_dasm_line.substr(comment_idx + 2);

                dasm_lines.emplace_back(line);
            }

            dasm_dirty = false;
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("VM")) {
                if (ImGui::MenuItem("Reload libv502", nullptr, nullptr))
                    lib_reload = true;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                ImGui::MenuItem("VSync", nullptr, &vsync);

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        glfwSwapInterval(vsync ? 1 : 0);

        bool manual_cycle = ImGui::Button("Step");

        ImGui::SameLine();

        ImGui::Checkbox("Auto Step", &auto_cycle);
        ImGui::InputInt("Step Interval (ms)", &cycle_interval);

        ImGui::InputInt("Substeps", &substeps);

        if (substeps < 1)
            substeps = 1;

        if (cycle_interval < 0)
            cycle_interval = 0;

        if (auto_cycle || manual_cycle) {
            cycle_wait += imguiIO.DeltaTime;

            if (cycle_wait >= (float)cycle_interval / 1000.0F || manual_cycle) {
                cycle_wait = 0;

                try {
                    for (int s = 0; s < substeps; s++)
                        v502_functions->v502_cycle_vm(vm);
                } catch (std::exception& err) {
                    call_stream << "Encountered exception while trying to cycle the CPU (check console for specifics!):\n" << err.what() << "\n" << std::endl;
                }

                // We refresh the image here instead because of performance
                uint8_t pixels[256 * 3];
                for (int p = 0; p < 256; p++) {
                    auto o = p * 3;
                    pixels[o] = vm->hunk[v502_functions->v502_make_word(image_page, p)];
                    pixels[o + 1] = vm->hunk[v502_functions->v502_make_word(image_page + 1, p)];
                    pixels[o + 2] = vm->hunk[v502_functions->v502_make_word(image_page + 2, p)];
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
            memory_stream << ((vm->flags >> t) & 1) << " ";
        }

        memory_stream << "|\n\n";
        memory_stream << "Registers: \n";

        memory_stream << "| IX = " << PAD_HEX_LO << +vm->index_x;
        memory_stream << " | IY = " << PAD_HEX_LO << +vm->index_y;
        memory_stream << " | AC = " << PAD_HEX_LO << +vm->accumulator;
        memory_stream << " | ST = " << PAD_HEX_LO << +vm->stack_ptr;
        memory_stream << " | PC = " << PAD_HEX << +vm->program_counter;
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
                int value = +vm->hunk[v502_functions->v502_make_word(page_number, idx)];
                memory_stream << PAD_HEX_LO << value << " ";
            }

            memory_stream << "\n";

        }
        memory_stream << std::flush;
        ImGui::Text("%s", memory_stream.str().c_str());

        ImGui::End();

        ImGui::Begin("Program");

        ImGui::InputTextWithHint("Bin File", "ex: test.bin", path_buf, 256);

        if (ImGui::Button("Load Bin")) {
            std::ifstream bin_file(path_buf, std::ifstream::binary);

            if (bin_file.is_open()) {
                bin_file.read(reinterpret_cast<char*>(vm->hunk), vm->hunk_length);
                v502_functions->v502_reset_vm(vm);

                dasm_dirty = true;
                bin_file.close();
            } else {
                call_stream << "Failed to load binary at '" << path_buf << "', does it exist? Do you have access to it?\n" << std::endl;
            }
        }

        memory_stream.str("");
        memory_stream.clear();
        memory_stream << std::hex << std::flush;

        ImGui::Text("Program Memory Slice:");

        auto lower = (vm->program_counter) / 16;
        auto upper = (vm->program_counter + 16) / 16;

        memory_stream << "              ";
        for (int x = lower; x < upper; x++) {
            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                memory_stream << (idx == vm->program_counter ? "vv" : "  ") << " ";
            }
        }
        memory_stream << "\n";

        for (int x = lower; x < upper; x++) {
            memory_stream << PAD_HEX << x * 16 << " -> ";
            memory_stream << PAD_HEX << ((x + 1) * 16) - 1 << ": ";

            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                int value = +vm->hunk[idx];
                memory_stream << PAD_HEX_LO << value << " ";
            }
            memory_stream << "\n";
        }

        memory_stream << std::endl;
        ImGui::Text("%s", memory_stream.str().c_str());

        ImGui::Text("Disassembly:");

        for (int l = 0; l < dasm_lines.size(); l++) {
            auto line = dasm_lines[l];

            uint32_t ahead = 0x0000; // How far ahead do we count it as "on this line"

            if (dasm_lines.size() > l + 1) {
                auto next = dasm_lines[l + 1];
                ahead = (next.location - 1) - line.location;
            }

            bool on_line = vm->program_counter <= line.location + ahead && vm->program_counter >= line.location;

            if (on_line)
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 213, 88, 255));

            ImGui::Text("%c\t0x%04x : %s", (on_line ? '>' : ' '), line.location, line.dasm.c_str());

            if (on_line)
                ImGui::PopStyleColor();
        }

        ImGui::End();

        ImGui::Begin("Simulation Stats");

        auto io = ImGui::GetIO();

        ImGui::Text("FPS: %f", io.Framerate);
        ImGui::Text("DeltaTime: %f", io.DeltaTime);

        ImGui::Spacing();

        ImGui::Text("If auto stepping at max speed:");
        ImGui::Text("Cycles (CPS) = %f", io.Framerate * substeps);

        ImGui::End();

        ImGui::Begin("Image");

        ImGui::InputInt("Image Page", &image_page);
        image_page = std::min(253, std::max(0, image_page));

        ImGui::Text("R = 0x%02x00 -> 0x%02xff", image_page, image_page);
        ImGui::Text("G = 0x%02x00 -> 0x%02xff", image_page + 1, image_page + 1);
        ImGui::Text("B = 0x%02x00 -> 0x%02xff", image_page + 2, image_page + 2);

        ImGui::Image(reinterpret_cast<void*>(image_buffer), ImVec2(128, 128));

        ImGui::End();

        ImGui::Begin("Symbols");

        for (int o = 0; o < 255; o++) {
            bool recognized = false;
            for (v502_assembler_symbol_t *sym = assembler_instance->symbol_stack; sym != nullptr; sym = sym->next) {
                if (v502_functions->v502_symbol_has_opcode(sym, o)) {
                    ImGui::Text("0x%02x : %s", o, sym->name);
                    recognized = true;
                    break;
                }
            }

            if (!recognized)
                ImGui::Text("0x%02x : Unknown", o);
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}