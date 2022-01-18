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

    DEFINE_OPERATION(SBC) {
        switch (code) {
            case OpCode::SBC_NOW:
                cpu->add_with_overflow(cpu->accumulator, cpu->next_byte(), true);
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

    // DEX and DEY are lopped into this
    DEFINE_OPERATION(DEC) {
        (code == DEX ? cpu->index_x : cpu->index_y) -= 1;
        return true;
    }

    // TXA and TYA are lopped into this
    DEFINE_OPERATION(TRA) {
        cpu->accumulator = (code == TXA ? cpu->index_x : cpu->index_y);
        return true;
    }

    // TAX and TAY are lopped into this
    DEFINE_OPERATION(TAR) {
        (code == TAX ? cpu->index_x : cpu->index_y) = cpu->accumulator;
        return true;
    }

    DEFINE_OPERATION(TXS) {
        cpu->stack_ptr = cpu->index_x;
        return true;
    }

    DEFINE_OPERATION(TSX) {
        cpu->index_x = cpu->stack_ptr;
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

    DEFINE_OPERATION(BPL) {
        if (!(cpu->flags & MOS6502::Flags::Negative)) {
            cpu->program_counter += (int8_t)cpu->next_byte();
            return false;
        }

        cpu->next_byte(); // Dispose of the jump
        return true;
    }

    DEFINE_OPERATION(BMI) {
        if (cpu->flags & MOS6502::Flags::Negative) {
            cpu->program_counter += (int8_t)cpu->next_byte();
            return false;
        }

        cpu->next_byte(); // Dispose of the jump
        return true;
    }

    DEFINE_OPERATION(BVC) {
        if (!(cpu->flags & MOS6502::Flags::Overflow)) {
            cpu->program_counter += (int8_t)cpu->next_byte();
            return false;
        }

        cpu->next_byte(); // Dispose of the jump
        return true;
    }

    DEFINE_OPERATION(BVS) {
        if (cpu->flags & MOS6502::Flags::Overflow) {
            cpu->program_counter += (int8_t)cpu->next_byte();
            return false;
        }

        cpu->next_byte(); // Dispose of the jump
        return true;
    }

    DEFINE_OPERATION(BCC) {
        if (!(cpu->flags & MOS6502::Flags::Carry)) {
            cpu->program_counter += (int8_t)cpu->next_byte();
            return false;
        }

        cpu->next_byte(); // Dispose of the jump
        return true;
    }

    DEFINE_OPERATION(BCS) {
        if (cpu->flags & MOS6502::Flags::Carry) {
            cpu->program_counter += (int8_t)cpu->next_byte();
            return false;
        }

        cpu->next_byte(); // Dispose of the jump
        return true;
    }

    DEFINE_OPERATION(BNE) {
        if (!(cpu->flags & MOS6502::Flags::Carry && cpu->flags & MOS6502::Flags::Zero)) {
            cpu->program_counter += (int8_t)cpu->next_byte();
            return false;
        }

        cpu->next_byte(); // Dispose of the jump
        return true;
    }

    DEFINE_OPERATION(BEQ) {
        if (cpu->flags & MOS6502::Flags::Carry && cpu->flags & MOS6502::Flags::Zero) {
            cpu->program_counter += (int8_t)cpu->next_byte();
            return false;
        }

        cpu->next_byte(); // Dispose of the jump
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
                byte_t offset = (code == LDA_ABS ? 0 : (code == LDA_X_ABS ? cpu->index_x : cpu->index_y));
                cpu->accumulator = cpu->get_at(cpu->next_word() + offset);
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

            case LDA_Y_IND: {
                cpu->accumulator = cpu->get_indirect(0x00, cpu->next_byte(), cpu->index_y);
                break;
            }
        }

        return true;
    }

    DEFINE_OPERATION(LDX) {
        switch (code) {
            case LDX_NOW:
                cpu->index_x = cpu->next_byte();
                break;

            case LDX_ABS:
            case LDX_Y_ABS: {
                byte_t offset = (code == LDX_ABS ? 0 : cpu->index_y);
                cpu->index_x = cpu->get_at(cpu->next_word() + offset);
                break;
            }

            case LDX_ZPG:
            case LDX_Y_ZPG: {
                byte_t offset = (code == LDX_ZPG ? 0 : cpu->index_y);
                cpu->index_x = cpu->get_at_page(0x00, cpu->next_byte() + offset);
                break;
            }
        }

        return true;
    }

    DEFINE_OPERATION(LDY) {
        switch (code) {
            case LDY_NOW:
                cpu->index_y = cpu->next_byte();
                break;

            case LDY_ABS:
            case LDY_X_ABS: {
                byte_t offset = (code == LDY_ABS ? 0 : cpu->index_x);
                cpu->index_y = cpu->get_at(cpu->next_word() + offset);
                break;
            }

            case LDY_ZPG:
            case LDY_X_ZPG: {
                byte_t offset = (code == LDY_ZPG ? 0 : cpu->index_x);
                cpu->index_y = cpu->get_at_page(0x00, cpu->next_byte() + offset);
                break;
            }
        }

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
        cpu->store_at_page(0x01, cpu->stack_ptr, 0x00);
        return true;
    }

    DEFINE_OPERATION(JSR) {
        cpu->store_at_page(0x01, cpu->stack_ptr--, (cpu->program_counter + 2));
        cpu->store_at_page(0x01, cpu->stack_ptr--, (cpu->program_counter + 2) >> 8);
        cpu->jump(cpu->next_word());

        return false;
    }

    DEFINE_OPERATION(RTS) {
        // For some reason the 6502 doesn't jump directly to the last spot, it's PC-1!
        byte_t l = cpu->get_at_page(0x01, cpu->stack_ptr + 2);
        byte_t h = cpu->get_at_page(0x01, cpu->stack_ptr + 1);

        cpu->jump_page(h, l);

        cpu->store_at_page(0x01, ++cpu->stack_ptr, 0x00);
        cpu->store_at_page(0x01, ++cpu->stack_ptr, 0x00);
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
            /* 2 */ OP_JSR, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_PLL, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 3 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 4 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_PSH, INVLID, INVLID, INVLID, OP_JMP, INVLID, INVLID, INVLID,
            /* 5 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 6 */ OP_RTS, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_PLL, OP_ADC, INVLID, INVLID, OP_JMP, INVLID, INVLID, INVLID,
            /* 7 */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* 8 */ INVLID, INVLID, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID, OP_DEC, INVLID, OP_TRA, INVLID, INVLID, OP_STA, INVLID, INVLID,
            /* 9 */ INVLID, INVLID, INVLID, INVLID, INVLID, OP_STA, INVLID, INVLID, OP_TRA, OP_STA, OP_TXS, INVLID, INVLID, OP_STA, INVLID, INVLID,
            /* A */ OP_LDY, OP_LDA, OP_LDX, INVLID, OP_LDY, OP_LDA, OP_LDX, INVLID, OP_TAR, OP_LDA, OP_TAR, INVLID, OP_LDY, OP_LDA, OP_LDX, INVLID,
            /* B */ INVLID, OP_LDA, INVLID, INVLID, OP_LDY, OP_LDA, OP_LDX, INVLID, INVLID, OP_LDA, OP_TSX, INVLID, OP_LDY, OP_LDA, OP_LDX, INVLID,
            /* C */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_INC, OP_CMP, OP_DEC, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* D */ INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* E */ OP_CPX, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, OP_INC, OP_SBC, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID,
            /* F */ OP_BEQ, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID, INVLID
    };
}