#ifndef V502_INSTRUCTION_CONTAINER_HPP
#define V502_INSTRUCTION_CONTAINER_HPP

#include <optional>
#include <stdint.h>
#include <vector>
#include <string>

using OptOpCode = std::optional<uint8_t>;

namespace V502 {
    enum CallingFlags : uint32_t {
        Indirect = 1,   // Use value at ADDR
        IndexedX = 2,   // ADDR + X
        IndexedY = 4,   // ADDR + Y
        ZeroPage = 8    // Implied 0x00 at the start
    };

    struct InstructionContainer {
        std::string symbol;
        OptOpCode zpg, x_zpg, y_zpg;
        OptOpCode abs, x_abs, y_abs;
        OptOpCode ind, x_ind, y_ind;
        OptOpCode now, only; // If only is set, we only use that one
        bool ind_word = false; // if ind_word = true, indirect calls use words instead of bytes

        OptOpCode get_code(uint32_t flags, bool word);

        static std::vector<InstructionContainer> containers;
    };
}

#endif
