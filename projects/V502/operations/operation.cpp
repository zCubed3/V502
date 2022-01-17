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

            case OpCode::JMP_IND: {
                word_t where = cpu->next_word();
                cpu->jump_page(cpu->system_memory->at(where + 1), cpu->system_memory->at(where));
                break;
            }
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

    // INX and INY are lopped into this
    DEFINE_OPERATION(INC) {
        (code == INX ? cpu->index_x : cpu->index_y) += 1;
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

    DEFINE_OPERATION(BEQ) {
        if (cpu->flags & MOS6502::Flags::Carry && cpu->flags & MOS6502::Flags::Zero) {
            cpu->program_counter = cpu->next_word();
            return false;
        }
        else
            cpu->next_word(); // Dispose of the jump

        return true;
    }

    DEFINE_OPERATION(LDA) {
        switch (code) {
            case LDA_NOW:
                cpu->accumulator = cpu->next_byte();
                break;

            case LDA_ABS:
            case LDA_X_ABS:
            case LDA_Y_ABS: {
                break;
            }

            case LDA_ZPG:
            case LDA_X_ZPG: {
                byte_t offset = (code == LDA_ZPG ? 0 : cpu->index_x);
                cpu->accumulator = cpu->get_at_page(0x00, cpu->next_byte() + offset);
                break;
            }

            case LDA_X_IND: {
                cpu->accumulator = cpu->get_indirect(0x00, cpu->next_byte() + cpu->index_x);
                break;
            }

            //case LDA_Y_IND:

        }
    }

    DEFINE_OPERATION(LDX) {
        cpu->index_x = cpu->next_byte();
        return true;
    }

    //
    // Push and pull, encapsulating both sets of operations
    //
    DEFINE_OPERATION(PSH) {
        cpu->store_at_page(0x01, cpu->stack_ptr--, (code == PLP ? cpu->flags : cpu->accumulator));
        return true;
    }

    DEFINE_OPERATION(PLL) {
        (code == PLP ? cpu->flags : cpu->accumulator) = cpu->get_at_page(0x01, ++cpu->stack_ptr);
        cpu->store_at_page(0x01, cpu->stack_ptr++, 0x00);
        return true;
    }

    DEFINE_OPERATION(BAD) {
        std::cerr << "0x" << std::hex << +code << std::dec << " is an invalid instruction!" << std::endl;
        throw std::runtime_error("Illegal instruction call! Either it doesn't exist or isn't defined yet!");
    }

    DEFINE_OPERATION(NOP) { return true; }

    const instruction_t OPERATIONS[256] = {
            /*      0       1       2       3       4       5       6       7       8       9       A       B       C       D       E       F   */
            /* 0 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_PSH, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 1 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_NOP, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 2 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_PLL, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 3 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 4 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_PSH, INVLID, INVLID, INVLID, OP_JMP, INVLID, INVLID, INVLID,
            /* 5 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 6 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_PLL, OP_ADC, INVLID, INVLID, OP_JMP, INVLID, INVLID, INVLID,
            /* 7 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 8 */ INVLID, INVLID, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID,
            /* 9 */ INVLID, INVLID, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID,
            /* A */ INVLID, OP_LDA, OP_LDX, INVLID, INVLID, OP_LDA, INVLID, INVLID, INVLID, OP_LDA, INVLID, INVLID, INVLID, OP_LDA, INVLID, INVLID,
            /* B */ INVLID, OP_LDA, INVLID, INVLID, INVLID, OP_LDA, INVLID, INVLID, INVLID, OP_LDA, INVLID, INVLID, INVLID, OP_LDA, INVLID, INVLID,
            /* C */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_INC, OP_CMP, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* D */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* E */ OP_CPX, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_INC, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* F */ OP_BEQ, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID
    };
}