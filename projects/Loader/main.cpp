#include <V502/v502.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip> // for setw and setfill

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#include <time.h>

#define PAD_HEX_LO std::setfill('0') << std::setw(2)
#define PAD_HEX std::setfill('0') << std::setw(4)

void zero_cursor() {
#ifdef _WIN32
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(output, { 0, 0 });
    std::cout << std::flush;
#endif

#ifdef __linux__
    std::cout << "\033[" << 0 << ";" << 0 << "H" << std::flush;
#endif
}

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
        std::cout << "Please provide a .bin file to execute!\nRun 'emu502 --help' to see possible arguments." << std::endl;
        return 1;
    }

    if (bin_path.empty()) {
        std::cerr << "No bin path was provided, please provide one using -b or --bin!" << std::endl;
        return 1;
    }

    v502_6502vm_createinfo_t createinfo {};
    createinfo.hunk_size = 0xFFFF + 1;

    v502_6502vm_t *cpu = v502_create_vm(&createinfo);

    std::ifstream bin_file(bin_path);

    if (!bin_file.is_open()) {
        std::cerr << "bin file not found at '" << bin_path << "'" << std::endl;
        return 1;
    }

    bin_file.read(reinterpret_cast<char*>(cpu->hunk), cpu->hunk_length);
    v502_reset_vm(cpu);

    bin_file.close();

    // This is the best it gets without ncurses!
    zero_cursor();
    std::cout << std::endl;
    for (int h = 0; h < 60; h++) {
        for (int x = 0; x < 512; x++)
            std::cout << " ";
        std::cout << std::endl;
    }
    zero_cursor();

    timespec wait = {};
    wait.tv_nsec = 1000; // 1mhz

    while (v502_cycle_vm(cpu)) {
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
                std::cout << "PRV";
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
                if (idx >= cpu->hunk_length)
                    break;

                int value = +cpu->hunk[idx];

                if (cpu->program_counter == idx) {
#if __linux__
                    std::cout << "\033[1;4;93m";
#endif

#if _WIN32
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | COMMON_LVB_UNDERSCORE);
#endif
                }

                if (value < 16)
                    std::cout << "0";

                std::cout << value;

                if (cpu->program_counter == idx) {
#if __linux__

                    std::cout << "\033[0m";
#endif

#if _WIN32
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
                }

                std::cout << " ";
            }

            std::cout << "\n";
        }

        std::cout << "\n";

        std::cout << "System Memory: \n";
        for (int x = 0; x < 16; x++) {
            std::cout << PAD_HEX << x * 16 << " -> ";
            std::cout << PAD_HEX << ((x + 1) * 16) - 1 << ": ";

            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                if (idx >= cpu->hunk_length)
                    break;

                int value = +cpu->hunk[idx];

                if (value < 16)
                    std::cout << "0";

                std::cout << value << " ";
            }

            std::cout << "\n";
        }

        std::cout << "\n";
        std::cout << "Stack Memory: \n";
        for (int x = 0; x < 16; x++) {
            // I hate C++ syntax at times...
            std::cout << "01" << PAD_HEX_LO << x * 16 << " -> ";
            std::cout << "01" << PAD_HEX_LO << ((x + 1) * 16) - 1 << ": ";

            for (int y = 0; y < 16; y++) {
                int idx = x * 16 + y;
                if (idx >= cpu->hunk_length)
                    break;

                int value = +cpu->hunk[v502_make_word(0x01, idx)];

                std::cout << PAD_HEX_LO << value << " ";
            }

            std::cout << "\n";
        }

#ifdef __linux__
        if (custom_time)
            usleep(interval * 1000);
        else
            nanosleep(&wait, nullptr);
#endif

#ifdef _WIN32
        if (custom_time)
            Sleep(interval);
        //else
            //throw std::runtime_error("Sorry windows doesn't support nanosleep() so you must provide an interval!");
#endif

        zero_cursor();
    }

    return 0;
}