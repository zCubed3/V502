#include <V502/components/mos6502.hpp>
#include <V502/components/memory.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip> // for setw and setfill

#include <unistd.h>
#include <time.h>

#define PAD_HEX_LO std::setfill('0') << std::setw(2)
#define PAD_HEX std::setfill('0') << std::setw(4)

void print_help() {
    std::cout << "Arguments: \n";
    std::cout << "\t-b or --bin, requires a value after, tells the program what binary file to load\n";
    std::cout << "\t-i or --interval, requires a number after, tells the program to wait the provided number of milliseconds\n";
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    std::string bin_path;
    bool custom_time = false;
    int interval = 0;

    if (argc > 1) {
        std::vector<std::string> args;

        for (int a = 1; a < argc; a++)
            args.emplace_back(std::string(argv[a]));

        bool need_input = false;
        std::string what_input = "";
        for (auto arg : args) {
            auto named = arg.find("--");

            if (need_input) {
                if (arg.find("-") != std::string::npos) {
                    std::cerr << what_input << " needs input but nothing was provided!" << std::endl;
                    return 1;
                }

                if (what_input == "bin" || what_input == "b") {
                    bin_path = arg;
                    need_input = false;
                }

                if (what_input == "interval" || what_input == "i") {
                    custom_time = true;
                    try {
                        interval = stoi(arg);
                        need_input = false;
                    } catch (std::exception err) {
                        std::cout << "Provided interval wasn't a valid number!" << std::endl;
                        std::cerr << err.what() << std::endl;
                        return 1;
                    }
                }
            } else {
                if (named != std::string::npos) {
                    std::string sub = arg.substr(2);

                    if (sub == "help") {
                        print_help();
                        return 0;
                    }

                    if (sub == "bin") {
                        need_input = true;
                        what_input = "bin";
                    }

                    if (sub == "interval") {
                        need_input = true;
                        what_input = "interval";
                    }
                } else {
                    auto shorthand = arg.find("-");

                    if (shorthand != std::string::npos) {
                        std::string sub = arg.substr(1);

                        for (auto ch : sub) {
                            if (ch == 'h') {
                                print_help();
                                return 0;
                            }

                            if (ch == 'b') {
                                need_input = true;
                                what_input = "b";
                            }

                            if (ch == 'i') {
                                need_input = true;
                                what_input = "i";
                            }
                        }
                    }
                }
            }
        }
    } else {
        std::cout << "Please provide a .bin file to execute!\nRun 'Emu502 --help' to see possible arguments." << std::endl;
        return 1;
    }

    if (bin_path.empty()) {
        std::cerr << "No bin path was provided, please provide one using -b or --bin!" << std::endl;
        return 1;
    }

    V502::MOS6502 *cpu = new V502::MOS6502();

    // In a real 6502 program memory and system memory are in the same space
    // TODO: Fix 6502 memory and combine it!
    V502::Memory *sys_memory = new V502::Memory(65535); // 64kb of memory
    cpu->system_memory = sys_memory;

    std::ifstream bin_file(bin_path);

    if (!bin_file.is_open()) {
        std::cerr << "bin file not found at '" << bin_path << "'" << std::endl;
        return 1;
    }

    sys_memory->copy_from(bin_file);
    cpu->reset();
    bin_file.close();

    // This is the best it gets without ncurses!
    std::cout << "\033[" << 0 << ";" << 0 << "H" << std::endl;
    for (int h = 0; h < 60; h++) {
        for (int x = 0; x < 512; x++)
            std::cout << " ";
        std::cout << std::endl;
    }
    std::cout << "\033[" << 0 << ";" << 0 << "H" << std::flush;

    timespec wait = {};
    wait.tv_nsec = 1000; // 1mhz

    while (cpu->cycle()) {
        std::cout << std::hex;

        std::cout << "[ Emu502 (6502 Simulator) - Powered by V502 ]\n\n";

        std::cout << "Flags: \n";
        std::cout << "| C Z I D - B V N |\n";
        std::cout << "| ";

        for (uint8_t t = 0; t < 8; t++) {
            std::cout << ((cpu->flags >> t) & 1) << " ";
        }

        std::cout << "|\n\n";
        std::cout << "Registers: \n";

        std::cout << "| IX = " << PAD_HEX_LO << +cpu->index_x;
        std::cout << " | IY = " << PAD_HEX_LO << +cpu->index_y;
        std::cout << " | AC = " << PAD_HEX_LO << +cpu->accumulator;
        std::cout << " | ST = " << PAD_HEX_LO << +cpu->stack_ptr;
        std::cout << " | PC = " << PAD_HEX << +cpu->program_counter;
        std::cout << " |        \n\n";

        std::cout << "Program Memory: \n";
        auto lower = (cpu->program_counter - 16) / 16;
        auto upper = (cpu->program_counter + 32) / 16;
        for (int x = lower; x < upper; x++) {
            if (x == lower) {
                std::cout << "BEF";
            } else if (x == upper - 1) {
                std::cout << "NXT";
            } else {
                std::cout << "CUR";
            }
            std::cout << ": ";

            if (x < 0) {
                std::cout << "NONE\n";
                continue;
            }

            std::cout << std::setfill('0') << std::setw(4) << x * 16 << " -> ";
            std::cout << std::setfill('0') << std::setw(4) << ((x + 1) * 16) - 1 << ": ";

            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                if (idx >= sys_memory->size())
                    break;

                int value = +sys_memory->at(idx);

                if (cpu->program_counter == idx)
                    std::cout<<"\033[1;4;93m";

                if (value < 16)
                    std::cout << "0";

                std::cout << value;

                if (cpu->program_counter == idx)
                    std::cout<<"\033[0m";

                std::cout << " ";
            }

            std::cout << "\n";
        }

        std::cout << "\n";

        std::cout << "System Memory: \n";
        for (int x = 0; x < 16; x++) { // TODO: Show more than the zero page
            // I hate C++ syntax at times...
            std::cout << std::setfill('0') << std::setw(4) << x * 16 << " -> ";
            std::cout << std::setfill('0') << std::setw(4) << ((x + 1) * 16) - 1 << ": ";

            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                if (idx >= sys_memory->size())
                    break;

                int value = +sys_memory->at(idx);

                if (value < 16)
                    std::cout << "0";

                std::cout << value << " ";
            }

            std::cout << "\n";
        }

        if (custom_time)
            usleep(interval * 1000);
        else
            nanosleep(&wait, nullptr);

        std::cout << "\033[" << 0 << ";" << 0 << "H" << std::flush;
    }

    return 0;
}