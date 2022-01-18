#include "instruction_container.hpp"

#include <operations/operation.hpp>

namespace V502 {
    OptOpCode InstructionContainer::get_code(uint32_t flags, bool word) {
        if (only)
            return only;

        if (word) {
            if (flags & CallingFlags::Indirect && ind_word) {
                if (flags & CallingFlags::IndexedX && x_ind)
                    return x_ind;

                if (flags & CallingFlags::IndexedY && y_ind)
                    return y_ind;

                if (ind)
                    return ind;
            } else {
                if (flags & CallingFlags::IndexedX && x_abs)
                    return x_abs;

                if (flags & CallingFlags::IndexedY && y_abs)
                    return y_abs;

                if (abs)
                    return abs;
            }
        } else {
            if (flags & CallingFlags::Indirect) {
                if (flags & CallingFlags::IndexedX && x_ind)
                    return x_ind;

                if (flags & CallingFlags::IndexedY && y_ind)
                    return y_ind;

                if (ind)
                    return ind;
            } else {
                if (flags & CallingFlags::ZeroPage) {
                    if (flags & CallingFlags::IndexedX && x_zpg)
                        return x_zpg;

                    if (flags & CallingFlags::IndexedY && y_zpg)
                        return y_zpg;

                    if (zpg)
                        return zpg;
                }

                if (flags & CallingFlags::Indirect && !ind_word) {
                    if (flags & CallingFlags::IndexedX && x_ind)
                        return x_ind;

                    if (flags & CallingFlags::IndexedY && y_ind)
                        return y_ind;

                    if (ind)
                        return ind;
                }

                if (now)
                    return now;
            }
        }

        return {};
    }

    // TODO: Containers for rest of instructions
    std::vector<InstructionContainer> InstructionContainer::containers {
        // { "", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, false },

        //
        // Stack operations
        //
        { "PHA", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, PHA, false },
        { "PHP", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, PHP, false },
        { "PLA", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, PLA, false },
        { "PLP", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, PLP, false },

        //
        // X Register
        //
        { "INX", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, INX, false },
        { "DEX", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, DEX, false },
        { "TAX", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, TAX, false },
        { "TXA", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, TXA, false },
        { "TSX", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, TSX, false },
        { "TXS", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, TXS, false },
        { "LDX", LDX_ZPG, {}, LDX_Y_ZPG, LDX_ABS, {}, LDX_Y_ABS, {}, {}, {}, LDX_NOW, {}, false },
        { "CPX", {}, {}, {}, {}, {}, {}, {}, {}, {}, CPX_NOW, {}, false },
        { "STX", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, false },

        //
        // Y Register
        //
        { "INY", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, INY, false },
        { "DEY", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, DEY, false },
        { "TAY", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, TAY, false },
        { "TYA", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, TYA, false },
        { "LDY", LDY_ZPG, LDY_X_ZPG, {}, LDY_ABS, LDY_X_ABS, {}, {}, {}, {}, LDY_NOW, {}, false },
        { "CPY", {}, {}, {}, {}, {}, {}, {}, {}, {}, LDY_NOW, {}, false },
        { "STY", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, false },

        //
        // A Register
        //
        { "STA", STA_ZPG, STA_X_ZPG, {}, STA_ABS, {}, {}, {}, {}, {}, {}, {}, false },
        { "LDA", LDA_ZPG, LDA_X_ZPG, {}, LDA_ABS, LDA_X_ABS, LDA_Y_ABS, {}, LDA_X_IND, LDA_Y_IND, LDA_NOW, {}, false },
        { "ADC", {}, {}, {}, {}, {}, {}, {}, {}, {}, ADC_NOW, {}, false },
        { "SBC", {}, {}, {}, {}, {}, {}, {}, {}, {}, SBC_NOW, {}, false },
        { "CMP", {}, {}, {}, {}, {}, {}, {}, {}, {}, CMP_NOW, {}, false },

        //
        // Flow
        //
        { "JMP", {}, {}, {}, JMP_ABS, {}, {}, JMP_IND, {}, {}, {}, {}, true },
        { "JSR", {}, {}, {}, JSR_ABS, {}, {}, {}, {}, {}, {}, {}, true },
        { "RTS", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, RTS, false },

        //
        // Branching
        //
        { "BPL", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, BPL, false, true },
        { "BMI", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, BMI, false, true },
        { "BVC", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, BVC, false, true },
        { "BVS", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, BVS, false, true },
        { "BCC", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, BCC, false, true },
        { "BCS", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, BCS, false, true },
        { "BNE", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, BNE, false, true },
        { "BEQ", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, BEQ, false, true },

        //
        // Misc
        //
        { "NOP", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, NOP, false },
    };
}