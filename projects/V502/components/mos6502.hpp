#ifndef V502_MOS6502_HPP
#define V502_MOS6502_HPP

#include <stdint.h>

// The 6502 is represented by its name because we might add the W65C02 some day!
namespace V502 {
    class MOS6502 {
    public:
        // The 6502 is laid out as close to its real counterpart as possible
        // We have a program counter, instruction decoder, etc...

        // TODO: Timing table and virtual CPU clock!

        // Some basic typedefs to make things stand out better

        typedef uint8_t register_t; // 8 bit register
        typedef uint16_t wregister_t; // 16 bit register

        //
        // Registers
        //
        wregister_t program_counter;

        register_t stack_ptr;
        register_t accumulator;
        register_t index_x;
        register_t index_y;
        register_t flags;

        //
        // Instruction decoding
        //

        // Adapted table from https://www.masswerk.at/6502/6502_instruction_set.html into a linear set of opcodes
        enum class Opcode {
            BRK_IMPL    = 0x00,
            ORA_X_IND   = 0x01,
            ORA_ZPG     = 0x05,
            ASL_ZPG     = 0x06,
            PHP_IMPL    = 0x08,
            ORA_NOW     = 0x09,
            ASL_A       = 0x0A,
            ORA_ABS     = 0x0D,
            ASL_ABS     = 0x0E,
            BPL_REL     = 0x10,
            ORA_Y_IND   = 0x11,
            ORA_X_ZPG   = 0x15,

            //TODO: 0x16 and above are missing except for a few
            LDA_NOW     = 0xA9,
            ADC_NOW     = 0x69,
            JMP_ABS     = 0x4C
        };

        // 16 x 16 matrix of instruction info

        // L is a placeholder to let the future me know something is undefined
#define L 0xFF

        // TODO: Fill in this table completely
        const uint8_t opcode_length_table[255] = {
                /* X    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
                /* 1 */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* 2 */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* 3 */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* 4 */ L, L, L, L, L, L, L, L, L, L, L, L, 2, L, L, L,
                /* 5 */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* 6 */ L, L, L, L, L, L, L, L, L, 1, L, L, L, L, L, L,
                /* 7 */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* 8 */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* 9 */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* A */ L, L, L, L, L, L, L, L, L, 1, L, L, L, L, L, L,
                /* B */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* C */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* D */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* E */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
                /* F */ L, L, L, L, L, L, L, L, L, L, L, L, L, L, L, L,
        };
    };
}

#endif
