#ifndef V502_6502_OPS_H
#define V502_6502_OPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../v502_types.h"

//
// MOS Instructions
//
typedef enum v502_MOS_OP {
    //TODO: More instructions

    // A good table of instructions https://www.masswerk.at/6502/6502_instruction_set.html

    //
    // X Register
    //
    v502_MOS_OP_INX         = 0xE8,
    v502_MOS_OP_DEX         = 0xCA,

    v502_MOS_OP_TAX         = 0xAA,
    v502_MOS_OP_TXA         = 0x8A,
    v502_MOS_OP_TXS         = 0x9A,
    v502_MOS_OP_TSX         = 0xBA,

    v502_MOS_OP_LDX_NOW     = 0xA2,
    v502_MOS_OP_LDX_ZPG     = 0xA6,
    v502_MOS_OP_LDX_Y_ZPG   = 0xB6,
    v502_MOS_OP_LDX_ABS     = 0xAE,
    v502_MOS_OP_LDX_Y_ABS   = 0xBE,

    v502_MOS_OP_CPX_NOW     = 0xE0,

    //
    // Y Register
    //
    v502_MOS_OP_INY         = 0xC8,
    v502_MOS_OP_DEY         = 0x88,

    v502_MOS_OP_TAY         = 0xA8,
    v502_MOS_OP_TYA         = 0x98,

    v502_MOS_OP_LDY_NOW     = 0xA0,
    v502_MOS_OP_LDY_ZPG     = 0xA4,
    v502_MOS_OP_LDY_X_ZPG   = 0xB4,
    v502_MOS_OP_LDY_ABS     = 0xAC,
    v502_MOS_OP_LDY_X_ABS   = 0xBC,

    //
    // A Register
    //
    v502_MOS_OP_STA_ZPG     = 0x85,
    v502_MOS_OP_STA_X_ZPG   = 0x95,
    v502_MOS_OP_STA_ABS     = 0x8D,
    v502_MOS_OP_STA_X_ABS   = 0x9D,
    v502_MOS_OP_STA_Y_ABS   = 0x99,

    v502_MOS_OP_PHA         = 0x48,
    v502_MOS_OP_PLA         = 0x68,

    v502_MOS_OP_LDA_NOW     = 0xA9,
    v502_MOS_OP_LDA_ZPG     = 0xA5,
    v502_MOS_OP_LDA_X_ZPG   = 0xB5,
    v502_MOS_OP_LDA_ABS     = 0xAD,
    v502_MOS_OP_LDA_X_ABS   = 0xBD,
    v502_MOS_OP_LDA_Y_ABS   = 0xB9,
    v502_MOS_OP_LDA_X_IND   = 0xA1,
    v502_MOS_OP_LDA_Y_IND   = 0xB1,

    v502_MOS_OP_ADC_NOW     = 0x69,
    v502_MOS_OP_ADC_ZPG     = 0x65,
    v502_MOS_OP_ADC_X_ZPG   = 0x75,
    v502_MOS_OP_ADC_ABS     = 0x6D,
    v502_MOS_OP_ADC_X_ABS   = 0x7D,
    v502_MOS_OP_ADC_Y_ABS   = 0x79,
    v502_MOS_OP_ADC_X_IND   = 0x61,
    v502_MOS_OP_ADC_Y_IND   = 0x71,

    v502_MOS_OP_AND_NOW     = 0x29,
    v502_MOS_OP_AND_ZPG     = 0x25,
    v502_MOS_OP_AND_X_ZPG   = 0x35,
    v502_MOS_OP_AND_ABS     = 0x2D,
    v502_MOS_OP_AND_X_ABS   = 0x3D,
    v502_MOS_OP_AND_Y_ABS   = 0x39,
    v502_MOS_OP_AND_X_IND   = 0x21,
    v502_MOS_OP_AND_Y_IND   = 0x31,

    v502_MOS_OP_SBC_NOW     = 0xE9,

    v502_MOS_OP_CMP_NOW     = 0xC9,

    //
    // State register
    //
    v502_MOS_OP_PHP         = 0x08,
    v502_MOS_OP_PLP         = 0x28,

    //
    // Flow
    //
    v502_MOS_OP_JMP_ABS     = 0x4C,
    v502_MOS_OP_JMP_IND     = 0x6C,

    v502_MOS_OP_JSR_ABS     = 0x20,
    v502_MOS_OP_RTS         = 0x60,

    //
    // Branching
    //
    v502_MOS_OP_BPL         = 0x10,
    v502_MOS_OP_BMI         = 0x30,
    v502_MOS_OP_BVC         = 0x50,
    v502_MOS_OP_BVS         = 0x70,
    v502_MOS_OP_BCC         = 0x90,
    v502_MOS_OP_BCS         = 0xB0,
    v502_MOS_OP_BNE         = 0xD0,
    v502_MOS_OP_BEQ         = 0xF0,

    //
    // Misc
    //
    v502_MOS_OP_NOP         = 0x1A // Not a real instruction, wastes a cycle though!
} v502_MOS_OP_E;

typedef enum v502_OP_STATE {
    V502_OP_STATE_FAILED, // Tells the VM something went wrong
    V502_OP_STATE_SUCCESS, // Tells the VM we passed
    V502_OP_STATE_SUCCESS_NO_COUNT // Tells the VM we passed but don't want to increment the program counter
} v502_OP_STATE_E;

//
// C <-> VM delegate
//
typedef struct v502_6502vm v502_6502vm_t;

typedef v502_OP_STATE_E(*v502_opfunc_t)(v502_6502vm_t*, v502_byte_t);

// Optional helper for defining opfuncs faster
#define v502_DEFINE_OPFUNC(NAME) v502_OP_STATE_E OP_##NAME(v502_6502vm_t* vm, v502_byte_t op)
// Example for TAX
/*
V502_OP_STATUS_E OP_TAX(v502_6502vm_t* vm, v502_byte_t op) {
    vm->index_x = vm->accumulator;
    return V502_OP_STATE_SUCCESS;
}
*/

void v502_populate_ops_vm(v502_6502vm_t* vm);

#ifdef __cplusplus
};
#endif

#endif
