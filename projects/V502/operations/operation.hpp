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
        TXS         = 0x9A,
        TSX         = 0xBA,

        LDX_NOW     = 0xA2,
        LDX_ZPG     = 0xA6,
        LDX_Y_ZPG   = 0xB6,
        LDX_ABS     = 0xAE,
        LDX_Y_ABS   = 0xBE,

        CPX_NOW     = 0xE0,

        //
        // Y Register
        //
        INY         = 0xC8,
        DEY         = 0x88,

        TAY         = 0xA8,
        TYA         = 0x98,

        LDY_NOW     = 0xA0,
        LDY_ZPG     = 0xA4,
        LDY_X_ZPG   = 0xB4,
        LDY_ABS     = 0xAC,
        LDY_X_ABS   = 0xBC,

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
        ADC_ZPG     = 0x65,
        ADC_X_ZPG   = 0x75,
        ADC_ABS     = 0x6D,
        ADC_X_ABS   = 0x7D,
        ADC_Y_ABS   = 0x79,
        ADC_X_IND   = 0x61,
        ADC_Y_IND   = 0x71,

        AND_NOW     = 0x29,
        AND_ZPG     = 0x25,
        AND_X_ZPG   = 0x35,
        AND_ABS     = 0x2D,
        AND_X_ABS   = 0x3D,
        AND_Y_ABS   = 0x39,
        AND_X_IND   = 0x21,
        AND_Y_IND   = 0x31,

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

        //
        // Branching
        //
        BPL         = 0x10,
        BMI         = 0x30,
        BVC         = 0x50,
        BVS         = 0x70,
        BCC         = 0x90,
        BCS         = 0xB0,
        BNE         = 0xD0,
        BEQ         = 0xF0,

        //
        // Misc
        //
        NOP         = 0x1A, // Not a real instruction but some assemblers provide it as a way to waste cycles
    };

    extern const instruction_t OPERATIONS[256];
}

#endif
