#include <V502/components/mos6502.hpp>
#include <V502/components/memory.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <V502/assembler/assembler.hpp>

void print_help() {
    std::cout << "Example: Asm502 -s test.s -o test.bin\n";
    std::cout << "Arguments: \n";
    std::cout << "\t-s or --src, requires a path after, provides the assembler with a source file\n";
    std::cout << "\t-o or --out, requires a path after, tells the assembler where to output to\n";
    std::cout << std::endl;
}

// Frontend for the assembler
int main(int argc, char** argv) {
    std::string source_path, out_path;

    if (argc > 3) {
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

                if (what_input == "src" || what_input == "s") {
                    source_path = arg;
                    need_input = false;
                }

                if (what_input == "out" || what_input == "o") {
                    out_path = arg;
                    need_input = false;
                }
            } else {
                if (named != std::string::npos) {
                    std::string sub = arg.substr(2);

                    if (sub == "help") {
                        print_help();
                        return 0;
                    }

                    if (sub == "src") {
                        need_input = true;
                        what_input = "s";
                    }

                    if (sub == "out") {
                        need_input = true;
                        what_input = "out";
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

                            if (ch == 's') {
                                need_input = true;
                                what_input = "s";
                            }

                            if (ch == 'o') {
                                need_input = true;
                                what_input = "o";
                            }
                        }
                    }
                }
            }
        }
    } else {
        std::cout << "Please provide a source and output file!\nPass --help to see possible arguments!" << std::endl;
        return 1;
    }

    V502::Assembler6502 *assembler = new V502::Assembler6502(source_path);
    std::vector<uint8_t> bytes = assembler->compile();

    std::ofstream out(out_path);
    out.write(reinterpret_cast<char*>(bytes.data()), bytes.size());
    out.close();

    return 0;
}