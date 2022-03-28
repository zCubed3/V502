#define V502_INCLUDE_ASSEMBLER
#include <v502/v502.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#if defined(__linux__) || defined(__FreeBSD__) || defined(__unix__)
//#define UNIX_LIKE
#include <unistd.h>
#endif

void print_help() {
    std::cout << "Example: asm502 -s test.s -o test.bin\n";
    std::cout << "Pipes also work! You're allowed to do 'cat asm.s | asm502 > asm.o'\n";
    std::cout << "Arguments: \n";
    std::cout << "\t-s or --src, requires a path after, provides the assembler with a source file\n";
    std::cout << "\t-o or --out, requires a path after, tells the assembler where to output to\n";
    std::cout << std::endl;
}

#define PIPE_INPUT_BUF_SIZE 1024

// Frontend for the assembler
int main(int argc, char** argv) {
    std::string source_path, out_path;

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
        if (!pipe_in && !pipe_out) {
            std::cerr << "Please provide a source and output file!\nPass --help to see possible arguments!"
                      << std::endl;
            return 1;
        }
    }

    if (out_path.empty() && !pipe_out) {
        std::cerr << "Please provide a output file!" << std::endl;
        return 1;
    }

    if (source_path.empty() && !pipe_in) {
        std::cerr << "Please provide a source file!" << std::endl;
        return 1;
    }

    v502_assembler_instance_t *assembler = v502_create_assembler();

    const char *source = NULL;
    std::string str_buf = "";

    if (!pipe_in)
        source = v502_load_source(source_path.c_str());
    else {
        char c = 0;
        while ((c = getc(stdin)) != EOF)
            str_buf += c;

        source = str_buf.c_str();
    }

    v502_binary_file_t *binary = v502_assemble_source(assembler, source);

    if (pipe_out) {
        fwrite(binary->bytes, binary->length, 1, stdout);
        fflush(stdout);
    } else {
        std::ofstream out(out_path);
        out.write(binary->bytes, binary->length);
        out.close();
    }

    std::cerr << std::endl;

    return 0;
}