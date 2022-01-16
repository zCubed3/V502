#include "operation.hpp"

#include <stdexcept>
#include <iostream>

#include <components/mos6502.hpp>
#include <components/memory.hpp>

#define DEFINE_OPERATION(NAME) bool OP_##NAME(int code, MOS6502* cpu)

namespace V502 {
    DEFINE_OPERATION(JMP) {
        switch (code) {
            case OpCode::JMP_ABS:
                cpu->jump(cpu->next_program_wide());
                break;
        }

        return false;
    }

    DEFINE_OPERATION(ADC) {
        switch (code) {
            case OpCode::ADC_NOW:
                cpu->accumulator += cpu->next_program();
                break;
        }

        return true;
    }

    DEFINE_OPERATION(STA) {
        switch (code) {
            case OpCode::STA_ZPG:
                cpu->store_at(cpu->next_program(), cpu->accumulator);
                break;
        }

        return true;
    }

    DEFINE_OPERATION(BAD) {
        std::cerr << "0x" << std::hex << code << std::dec << " is an invalid instruction!" << std::endl;
        throw std::runtime_error("Illegal instruction call! Either it doesn't exist or isn't defined yet!");
    }

#define INVLID OP_BAD

    const instruction_t OPERATIONS[256] = {
            /*      0       1       2       3       4       5       6       7       8       9       A       B       C       D       E       F   */
            /* 0 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 1 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 2 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 3 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 4 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_JMP, INVLID, INVLID, INVLID,
            /* 5 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 6 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_ADC, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 7 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 8 */ INVLID, INVLID, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 9 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* A */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* B */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* C */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* D */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* E */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* F */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID
    };
}