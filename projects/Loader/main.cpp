#include <V502/components/mos6502.hpp>
#include <V502/components/memory.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <unistd.h>
#include <time.h>

void print_help() {
    std::cout << "Arguments: \n";
    std::cout << "\t-b or --bin, requires a value after, tells the program what binary file to load\n";
    std::cout << "\t-d or --debug, tells the program to increase verbosity (Does nothing right now)\n";
    std::cout << "\t-i or --interval, requires a number after, tells the program to wait the provided number of milliseconds\n";
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    std::string binpath;
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
                    binpath = arg;
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

    V502::MOS6502 *cpu = new V502::MOS6502();

    V502::Memory *sys_memory = new V502::Memory(255);
    cpu->system_memory = sys_memory;

    if (binpath.empty()) {
        std::cerr << "No bin path was provided, please provide one using -b or --bin!" << std::endl;
        return 1;
    }

    std::ifstream binfile(binpath);

    if (!binfile.is_open()) {
        std::cerr << "bin file not found at '" << binpath << "'" << std::endl;
        return 1;
    }

    V502::Memory *prog_memory = new V502::Memory(binfile);
    cpu->program_memory = prog_memory;

    binfile.close();

    std::cout << "Binary data: " << std::endl;
    for (auto d = 0; d < prog_memory->size(); d++) {
        std::cout << std::hex << +prog_memory->at(d) << " ";

        if (d % 16 == 0 && d != 0)
            std::cout << std::endl;
    }

    std::cout << std::dec << std::endl;
    timespec wait = {};

    wait.tv_nsec = 1000; // 1mhz

    while (cpu->cycle()) {
        std::cout << std::hex;
        std::cout << "(X: " << +cpu->index_x << ") ";
        std::cout << "0 -> 15: ";
        for (int x = 0; x < 16; x++) {
            std::cout << +sys_memory->at(x) << " ";
        }
        std::cout << "\r" << std::flush;

        if (custom_time)
            usleep(interval * 1000);
        else
            nanosleep(&wait, nullptr);

        //for (int x = 0; x < 8; x++) {
        //    printf("%i", (cpu->flags << x) & 1);
        //}

        //printf("\n");
    }

    return 0;
}