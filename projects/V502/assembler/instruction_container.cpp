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

    std::vector<InstructionContainer> InstructionContainer::containers {
        // { "", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, false },
        { "JMP", {}, {}, {}, JMP_ABS, {}, {}, {}, {}, {}, {}, {}, true },
        { "STA", STA_ZPG, STA_X_ZPG, {}, STA_ABS, {}, {}, {}, {}, {}, {}, {}, false },
        { "INX", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, INX, false },
        { "INY", {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, INY, false },
        { "CPX", {}, {}, {}, {}, {}, {}, {}, {}, {}, CPX_NOW, {}, false },
        { "BEQ", {}, {}, {}, {}, {}, {}, {}, {}, {}, BEQ, {}, false },
        { "ADC", {}, {}, {}, {}, {}, {}, {}, {}, {}, ADC_NOW, {}, false },
        { "LDX", {}, {}, {}, {}, {}, {}, {}, {}, {}, LDX_NOW, {}, false },
    };
}