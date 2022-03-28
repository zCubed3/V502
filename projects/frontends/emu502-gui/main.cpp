#include <iostream>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip> // for setw and setfill

#include <glad/glad.h>
#include <SDL2/SDL.h>

#include <imgui.h>
#include <backends/imgui_impl_opengl2.h>
#include <backends/imgui_impl_sdl.h>

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
            FreeLibrary(module);
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

#ifdef WIN32
        auto fptr = (ftpr_get_function_table)GetProcAddress(module, "v502_get_function_table");
        return fptr();
#endif
    }
};

void DrawOpcodeDebugWindow(v502_function_table_t* v502_functions, v502_6502vm_t* vm, v502_assembler_instance_t* assembler_instance) {
    if (!ImGui::Begin("Opcode Debugger")) {
        ImGui::End();
        return;
    }

    ImGui::BeginTable("opcode_example_table", 2, ImGuiTableFlags_Borders);

    ImGui::TableSetupColumn("SYM", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("WHAT", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("IMM");
    ImGui::TableNextColumn();
    ImGui::Text("Immediate (only 1 opcode)");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("NOW");
    ImGui::TableNextColumn();
    ImGui::Text("Now");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("ZPG");
    ImGui::TableNextColumn();
    ImGui::Text("Zero Page");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("ZPG_X");
    ImGui::TableNextColumn();
    ImGui::Text("Zero Page X Indexed");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("ZPG_Y");
    ImGui::TableNextColumn();
    ImGui::Text("Zero Page Y Indexed");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("ABS");
    ImGui::TableNextColumn();
    ImGui::Text("Absolute");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("ABS_X");
    ImGui::TableNextColumn();
    ImGui::Text("Absolute X Indexed");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("ABS_Y");
    ImGui::TableNextColumn();
    ImGui::Text("Absolute Y Indexed");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("IND");
    ImGui::TableNextColumn();
    ImGui::Text("Indirect");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("IND_X");
    ImGui::TableNextColumn();
    ImGui::Text("Indirect X Indexed");

    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    ImGui::Text("IND_Y");
    ImGui::TableNextColumn();
    ImGui::Text("Indirect Y Indexed");

    ImGui::EndTable();

    auto fallback_func = v502_functions->v502_get_fallback_func();
    for (v502_assembler_symbol_t *sym = assembler_instance->symbol_stack; sym != nullptr; sym = sym->next) {
        if (ImGui::TreeNode(sym->name)) {
            // This is SUPER hacky, but we can check which opcodes are defined since in memory the symbols are technically just int arrays!
            bool is_loner = sym->only != 0xFFFF;

            ImGui::PushID(sym->name);
            ImGui::BeginTable("##symbol_table", 3, ImGuiTableFlags_Borders);

            ImGui::TableSetupColumn("SYM", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("CODE", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableSetupColumn("STATE", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            if (is_loner) {
                ImGui::TableNextRow();

                bool is_missing = vm->opfuncs[sym->only] == fallback_func;

                ImGui::TableNextColumn();
                ImGui::Text("IMM");
                ImGui::TableNextColumn();

                ImGui::Text("%02x", sym->only);
                ImGui::TableNextColumn();

                if (is_missing) {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(244, 67, 54, 255));
                    ImGui::Text("NO IMPL");
                    ImGui::PopStyleColor();
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(76, 175, 80, 255));
                    ImGui::Text("HAS IMPL");
                    ImGui::PopStyleColor();
                }
            } else {
                uint16_t *opcode = &sym->zpg;

                const char* sym_names[10] = {
                        "ZPG",
                        "ZPG_X",
                        "ZPG_Y",
                        "ABS",
                        "ABS_X",
                        "ABS_Y",
                        "IND",
                        "IND_X",
                        "IND_Y",
                        "NOW",
                };

                bool all_missing = true;
                for (int o = 0; o < 10; o++) {
                    if (opcode[o] == 0xFFFF)
                        continue;

                    all_missing = false;

                    if (o != 9 && o != 0)
                        ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", sym_names[o]);
                    ImGui::TableNextColumn();

                    // Check if this is missing from the VM
                    bool is_missing = vm->opfuncs[opcode[o]] == fallback_func;

                    ImGui::Text("%02x", opcode[o]);
                    ImGui::TableNextColumn();

                    if (is_missing) {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(244, 67, 54, 255));
                        ImGui::Text("NO IMPL");
                        ImGui::PopStyleColor();
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(76, 175, 80, 255));
                        ImGui::Text("HAS IMPL");
                        ImGui::PopStyleColor();
                    }
                }

                if (all_missing) {
                    for (int x = 0; x < 3; x++) {
                        ImGui::TableNextColumn();
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(244, 67, 54, 255));
                        ImGui::Text("MISSING ALL");
                        ImGui::PopStyleColor();
                    }
                }
            }

            ImGui::EndTable();
            ImGui::PopID();

            ImGui::TreePop();
        }
    }

    ImGui::End();
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);

    SDL_Window *window = SDL_CreateWindow("V502 GUI", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    SDL_GLContext sdl_context = SDL_GL_CreateContext(window);

    if (!gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress)) {
        throw std::runtime_error("GLAD failed to initialize!");
    }

    V502Library lib {};
    lib.LoadLibrary();

    auto v502_functions = lib.GetTable();

    v502_6502vm_createinfo_t createinfo {};
    createinfo.hunk_size = 0xFFFF + 1;

    v502_6502vm_t *vm = v502_functions->v502_create_vm(&createinfo);

    v502_assembler_instance_t* assembler_instance = v502_functions->v502_create_assembler();

    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(window, sdl_context);
    ImGui_ImplOpenGL2_Init();

    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::StyleColorsDark(&style);

    ImGuiIO &imguiIO = ImGui::GetIO();
    imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    //imguiIO.Fonts->AddFontFromFileTTF("font.ttf", 14);

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

    bool long_flag_name = false, long_reg_name = false;

    bool should_close = false;
    bool resized = false;
    SDL_Event sdl_event;

    while (!should_close) {
        while (SDL_PollEvent(&sdl_event) != 0) {
            if (sdl_event.type == SDL_QUIT)
                should_close = false;

            if (sdl_event.type == SDL_WINDOWEVENT) {
                if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED)
                    resized = true;
            }

            ImGui_ImplSDL2_ProcessEvent(&sdl_event);
        }

        glClearColor(0.1F, 0.1F, 0.1F, 1.0F);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        //ImGui::ShowDemoWindow();
        //ImGui::ShowStyleEditor();

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

                if (ImGui::MenuItem("Refresh DASM", nullptr, nullptr))
                    dasm_dirty = true;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                ImGui::MenuItem("Auto Step", nullptr, &auto_cycle);

                ImGui::MenuItem("Long Flag Name", nullptr, &long_flag_name);
                ImGui::MenuItem("Long Register Name", nullptr, &long_reg_name);

                ImGui::MenuItem("VSync", nullptr, &vsync);

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        SDL_GL_SetSwapInterval(vsync ? 1 : 0);

        ImGui::InputInt("Step Interval (ms)", &cycle_interval);
        ImGui::InputInt("Substeps", &substeps);

        if (substeps < 1)
            substeps = 1;

        if (cycle_interval < 0)
            cycle_interval = 0;

        ImGui::Text("Flags:");
        ImGui::BeginTable("v502_flag_table", 8, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders, ImVec2(long_flag_name ? 390 : 180, 10));

        const char* flag_names[8] = {
                "Carry",
                "Zero",
                "Interrupt",
                "Decimal",
                "Break",
                "---",
                "Overflow",
                "Negative"
        };

        const char* flag_ids[8] = {
                "C",
                "Z",
                "I",
                "D",
                "B",
                "-",
                "O",
                "N"
        };

        if (long_flag_name) {
            for (auto &flag_name: flag_names)
                ImGui::TableSetupColumn(flag_name, ImGuiTableColumnFlags_WidthFixed);
        } else {
            for (auto &flag_char: flag_ids)
                ImGui::TableSetupColumn(flag_char, ImGuiTableColumnFlags_WidthFixed);
        }

        ImGui::TableHeadersRow();

        ImGui::TableNextRow();

        for (uint8_t t = 0; t < 8; t++) {
            ImGui::TableNextColumn();
            ImGui::Text("%02x", ((vm->flags >> t) & 1));
        }

        ImGui::EndTable();

        ImGui::Text("Registers:");
        ImGui::BeginTable("v502_register_table", 5, ImGuiTableFlags_Borders, ImVec2(long_reg_name ? 390 : 130, 10));

        const char* reg_short_names[5] = { "IX", "IY", "AC", "ST", "PC" };
        const char* reg_names[5] = { "Index X", "Index Y", "Accumulator", "Stack Pointer", "Program Counter" };

        if (long_reg_name) {
            for (auto &reg_name: reg_names)
                ImGui::TableSetupColumn(reg_name, ImGuiTableColumnFlags_WidthFixed);
        } else {
            for (auto &reg_name: reg_short_names)
                ImGui::TableSetupColumn(reg_name, ImGuiTableColumnFlags_WidthFixed);
        }

        ImGui::TableHeadersRow();

        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::Text("%02x", +vm->index_x);

        ImGui::TableNextColumn();
        ImGui::Text("%02x", +vm->index_y);

        ImGui::TableNextColumn();
        ImGui::Text("%02x", +vm->accumulator);

        ImGui::TableNextColumn();
        ImGui::Text("%02x", +vm->stack_ptr);

        ImGui::TableNextColumn();
        ImGui::Text("%04x", +vm->program_counter);

        ImGui::EndTable();

        bool manual_cycle = ImGui::Button("Step");

        ImGui::End();

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

        ImGui::Begin("Memory");
        ImGui::InputInt("Page", &page_number);

        if (page_number < 0x00)
            page_number = 0x00;

        if (page_number > 0xFF)
            page_number = 0xFF;

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        for (int y = 0; y < 16; y++) {
            v502_word_t page = v502_functions->v502_make_word(page_number, y * 16);
            v502_word_t end = page + 15;
            ImGui::Text("%04x -> %04x: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
                        page, end,
                        vm->hunk[page], vm->hunk[page + 1], vm->hunk[page + 2], vm->hunk[page + 3],
                        vm->hunk[page + 4], vm->hunk[page + 5], vm->hunk[page + 6], vm->hunk[page + 7],
                        vm->hunk[page + 8], vm->hunk[page + 9], vm->hunk[page + 10], vm->hunk[page + 11],
                        vm->hunk[page + 12], vm->hunk[page + 13], vm->hunk[page + 14], vm->hunk[page + 15]
            );
        }
        ImGui::PopStyleVar();

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

        ImGui::Spacing();
        ImGui::Text("Program Memory Slice:");

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 0));

        auto lower = (vm->program_counter) / 16;
        auto upper = (vm->program_counter + 16) / 16;

        ImGui::Text(" \t        ");
        for (int x = lower; x < upper; x++) {
            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                ImGui::SameLine();
                ImGui::Text("%s", idx == vm->program_counter ? "vv" : "  ");
            }
        }

        ImGui::Text(" \t0x%04x: ", lower * 16);

        for (int x = lower; x < upper; x++) {
            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                bool on_line = vm->program_counter == idx;

                if (on_line)
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 213, 88, 255));

                ImGui::SameLine();
                ImGui::Text("%02x", vm->hunk[idx]);

                if (on_line)
                    ImGui::PopStyleColor();
            }
        }

        ImGui::PopStyleVar();

        ImGui::Spacing();
        ImGui::Spacing();

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

        ImGui::Begin("Memory Visualizer");

        ImGui::InputInt("Image Page", &image_page);
        image_page = std::min(253, std::max(0, image_page));

        ImGui::Text("R = 0x%02x00 -> 0x%02xff", image_page, image_page);
        ImGui::Text("G = 0x%02x00 -> 0x%02xff", image_page + 1, image_page + 1);
        ImGui::Text("B = 0x%02x00 -> 0x%02xff", image_page + 2, image_page + 2);

        ImGui::Image(reinterpret_cast<void*>(image_buffer), ImVec2(128, 128));

        ImGui::End();

        DrawOpcodeDebugWindow(v502_functions, vm, assembler_instance);

        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }
}