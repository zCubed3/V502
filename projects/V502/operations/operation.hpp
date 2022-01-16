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

        NOP         = 0x1A,

        INX         = 0xE8,

        INY         = 0xC8,

        LDA_NOW     = 0xA9,

        LDX_NOW     = 0xA2,

        ADC_NOW     = 0x69,

        JMP_ABS     = 0x4C,

        STA_ZPG     = 0x85,
        STA_X_ZPG   = 0x95,
        STA_ABS     = 0x8D,
        STA_X_ABS   = 0x9D,
        STA_Y_ABS   = 0x99,

        CMP_NOW     = 0xC9,

        CPX_NOW     = 0xE0,

        BEQ         = 0xF0,
    };

    extern const instruction_t OPERATIONS[256];
}

#endif
