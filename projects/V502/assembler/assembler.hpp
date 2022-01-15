#ifndef V502_ASSEMBLER_HPP
#define V502_ASSEMBLER_HPP

#include <string>
#include <fstream>
#include <vector>

namespace V502 {
    // A basic assembler class, provide it a file and it'll assemble it into an opcode representation
    // TODO: More complex assembly (labels and such)
    class Assembler6502 {
    public:
        Assembler6502(std::string path);

        std::string assembly = "";
        std::vector<uint8_t> Compile();
    };
}

#endif
