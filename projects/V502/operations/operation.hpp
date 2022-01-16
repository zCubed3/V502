#ifndef V502_OPERATION_HPP
#define V502_OPERATION_HPP

#include <stdint.h>

namespace V502 {
    class MOS6502;

    typedef bool(*instruction_t)(int, MOS6502*);
    typedef uint8_t register_t;

    //
    // Instructions
    //

    // TODO: Timing info might be needed to emulate a system clock
    // Adapted table from https://www.masswerk.at/6502/6502_instruction_set.html into a linear array of opcodes
    enum OpCode : register_t {
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
        JMP_ABS     = 0x4C,
        STA_ZPG     = 0x85,
        STA         = 0x8D,
    };

    extern const instruction_t OPERATIONS[256];
}

#endif
