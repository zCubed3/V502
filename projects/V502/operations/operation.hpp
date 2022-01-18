#ifndef V502_OPERATION_HPP
#define V502_OPERATION_HPP

#include <stdint.h>
#include <v502common.hpp>

namespace V502 {
    class MOS6502;

    typedef bool(*instruction_t)(byte_t, MOS6502*);

    //
    // Instructions
    //

    // TODO: Timing info might be needed to emulate a system clock
    // Adapted table from https://www.masswerk.at/6502/6502_instruction_set.html into a linear array of opcodes

    // X_IDX = use the X index
    // Y_IDX = use the Y index
    // ZPG = use only the 0x00 page
    // X_ZPG = ZPG + X Index (wraps)
    // Y_ZPG = ZPG + Y Index (wraps)
    // ABS = use the following address
    enum OpCode : byte_t {
        //TODO: More instructions

        //
        // X Register
        //
        INX         = 0xE8,
        DEX         = 0xCA,

        TAX         = 0xAA,
        TXA         = 0x8A,

        LDX_NOW     = 0xA2,

        CPX_NOW     = 0xE0,

        //
        // Y Register
        //
        INY         = 0xC8,
        DEY         = 0x88,

        TAY         = 0xA8,
        TYA         = 0x98,

        //
        // A Register
        //
        STA_ZPG     = 0x85,
        STA_X_ZPG   = 0x95,
        STA_ABS     = 0x8D,
        STA_X_ABS   = 0x9D,
        STA_Y_ABS   = 0x99,

        PHA         = 0x48,
        PLA         = 0x68,

        LDA_NOW     = 0xA9,
        LDA_ZPG     = 0xA5,
        LDA_X_ZPG   = 0xB5,
        LDA_ABS     = 0xAD,
        LDA_X_ABS   = 0xBD,
        LDA_Y_ABS   = 0xB9,
        LDA_X_IND   = 0xA1,
        LDA_Y_IND   = 0xB1,

        ADC_NOW     = 0x69,

        SBC_NOW     = 0xE9,

        CMP_NOW     = 0xC9,

        //
        // State register
        //
        PHP         = 0x08,
        PLP         = 0x28,

        //
        // Flow
        //
        JMP_ABS     = 0x4C,
        JMP_IND     = 0x6C,

        JSR_ABS     = 0x20,
        RTS         = 0x60,

        BEQ         = 0xF0,

        //
        // Misc
        //
        NOP         = 0x1A, // Not a real instruction but some assemblers provide it as a way to waste cycles
    };

    extern const instruction_t OPERATIONS[256];
}

#endif
