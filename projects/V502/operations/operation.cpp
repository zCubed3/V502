#include "operation.hpp"

#include <stdexcept>
#include <iostream>

#include <components/mos6502.hpp>
#include <components/memory.hpp>

#define DEFINE_OPERATION(NAME) bool OP_##NAME(byte_t code, MOS6502* cpu)
#define INVLID OP_BAD

namespace V502 {
    DEFINE_OPERATION(JMP) {
        switch (code) {
            case OpCode::JMP_ABS:
                cpu->jump(cpu->next_word());
                break;
        }

        return false;
    }

    DEFINE_OPERATION(ADC) {
        switch (code) {
            case OpCode::ADC_NOW:
                cpu->add_with_overflow(cpu->accumulator, cpu->next_byte());
                break;
        }

        return true;
    }

    DEFINE_OPERATION(STA) {
        switch (code) {
            case OpCode::STA_X_ZPG:
            case OpCode::STA_ZPG:
                cpu->store_at_page(0, cpu->next_byte() + (code == STA_X_ZPG ? cpu->index_x : 0), cpu->accumulator);
                break;

            case OpCode::STA_ABS:
            case OpCode::STA_X_ABS:
            case OpCode::STA_Y_ABS: {
                cpu->store_at(cpu->next_word() + (code == STA_ABS ? 0 : (code == STA_X_ABS ? cpu->index_x : cpu->index_y)), cpu->accumulator);
                break;
            }
        }

        return true;
    }

    DEFINE_OPERATION(INX) {
        cpu->index_x += 1;
        return true;
    }

    DEFINE_OPERATION(INY) {
        cpu->index_y += 1;
        return true;
    }

    DEFINE_OPERATION(CMP) {
        switch (code) {
            case CMP_NOW:
                cpu->compare(cpu->accumulator, cpu->next_byte());
                break;
        }

        return true;
    }

    DEFINE_OPERATION(CPX) {
        switch (code) {
            case CPX_NOW:
                cpu->compare(cpu->index_x, cpu->next_byte());
                break;
        }

        return true;
    }

    // TODO: Not actually similar to the 6502 way of doing it!
    // TODO: Relative jumps and other stuff
    DEFINE_OPERATION(BEQ) {
        if (cpu->flags & MOS6502::Flags::Carry && cpu->flags & MOS6502::Flags::Zero) {
            cpu->program_counter += cpu->next_byte();
            return false;
        }
        else
            cpu->next_byte(); // Dispose of the jump

        return true;
    }

    DEFINE_OPERATION(LDX) {
        cpu->index_x = cpu->next_byte();
        return true;
    }

    DEFINE_OPERATION(BAD) {
        std::cerr << "0x" << std::hex << +code << std::dec << " is an invalid instruction!" << std::endl;
        throw std::runtime_error("Illegal instruction call! Either it doesn't exist or isn't defined yet!");
    }

    DEFINE_OPERATION(NOP) { return true; }

    const instruction_t OPERATIONS[256] = {
            /*      0       1       2       3       4       5       6       7       8       9       A       B       C       D       E       F   */
            /* 0 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 1 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_NOP, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 2 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 3 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 4 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_JMP, INVLID, INVLID, INVLID,
            /* 5 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 6 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_ADC, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 7 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 8 */ INVLID, INVLID, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID,
            /* 9 */ INVLID, INVLID, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID,
            /* A */ INVLID, INVLID, OP_LDX, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* B */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* C */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_INY, OP_CMP, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* D */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* E */ OP_CPX, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_INX, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* F */ OP_BEQ, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID
    };
}