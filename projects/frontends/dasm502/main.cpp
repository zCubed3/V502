#define V502_INCLUDE_ASSEMBLER
#include <v502/v502.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#if defined(__linux__) || defined(__FreeBSD__) || defined(__unix__)
#define UNIX_LIKE
#include <unistd.h>
#endif

void print_help() {
    std::cout << "Example: asm502 -s test.s -o test.bin\n";
    std::cout << "Pipes also work! You're allowed to do 'cat asm.s | asm502 > asm.o'\n";
    std::cout << "Arguments: \n";
    std::cout << "\t-b or --bin, requires a path after, provides the disassembler with a path to a binary\n";
    std::cout << "\t-o or --out, requires a path after, tells the assembler where to output to\n";
    std::cout << std::endl;
}

#define PIPE_INPUT_BUF_SIZE 1024

// Frontend for the assembler
int main(int argc, char** argv) {
    std::string binary_path, out_path;

    bool pipe_in = false, pipe_out = false;
#ifdef UNIX_LIKE
    pipe_in = !isatty(fileno(stdin));
    pipe_out = !isatty(fileno(stdout));
#endif

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
                    binary_path = arg;
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

                    if (sub == "bin") {
                        need_input = true;
                        what_input = "bin";
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

                            if (ch == 'b') {
                                need_input = true;
                                what_input = "b";
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
        if (!pipe_in && !pipe_out) {
            std::cerr << "Please provide a binary and output file!\nPass --help to see possible arguments!"
                      << std::endl;
            return 1;
        }
    }

    if (out_path.empty() && !pipe_out) {
        std::cerr << "Please provide a output file!" << std::endl;
        return 1;
    }

    if (binary_path.empty() && !pipe_in) {
        std::cerr << "Please provide a binary file!" << std::endl;
        return 1;
    }

    v502_assembler_instance_t *assembler = v502_create_assembler();

    v502_binary_file_t binary {};
    binary.length = 0xFFFF + 1;
    binary.bytes = new char[binary.length];

    if (!pipe_in) {
        std::ifstream binary_file = std::ifstream(binary_path.c_str(), std::ifstream::binary);
        binary_file.read(binary.bytes, binary.length);
        binary_file.close();
    } else {
        fread(binary.bytes, binary.length, 1, stdin);
    }

    std::string disassembly = v502_disassemble_binary(assembler, &binary);

    if (pipe_out) {
        fwrite(disassembly.c_str(), disassembly.length(), 1, stdout);
        fflush(stdout);
    } else {
        std::ofstream out(out_path);
        out << disassembly << std::flush;
        out.close();
    }

    return 0;
}