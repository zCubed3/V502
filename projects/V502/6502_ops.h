#ifndef V502_6502_OPS_H
#define V502_6502_OPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "6502_types.h"

//
// MOS Instructions
//
typedef enum V502_MOS_OP {
    //TODO: More instructions

    //
    // X Register
    //
    V502_MOS_OP_INX         = 0xE8,
    V502_MOS_OP_DEX         = 0xCA,

    V502_MOS_OP_TAX         = 0xAA,
    V502_MOS_OP_TXA         = 0x8A,
    V502_MOS_OP_TXS         = 0x9A,
    V502_MOS_OP_TSX         = 0xBA,

    V502_MOS_OP_LDX_NOW     = 0xA2,
    V502_MOS_OP_LDX_ZPG     = 0xA6,
    V502_MOS_OP_LDX_Y_ZPG   = 0xB6,
    V502_MOS_OP_LDX_ABS     = 0xAE,
    V502_MOS_OP_LDX_Y_ABS   = 0xBE,

    V502_MOS_OP_CPX_NOW     = 0xE0,

    //
    // Y Register
    //
    V502_MOS_OP_INY         = 0xC8,
    V502_MOS_OP_DEY         = 0x88,

    V502_MOS_OP_TAY         = 0xA8,
    V502_MOS_OP_TYA         = 0x98,

    V502_MOS_OP_LDY_NOW     = 0xA0,
    V502_MOS_OP_LDY_ZPG     = 0xA4,
    V502_MOS_OP_LDY_X_ZPG   = 0xB4,
    V502_MOS_OP_LDY_ABS     = 0xAC,
    V502_MOS_OP_LDY_X_ABS   = 0xBC,

    //
    // A Register
    //
    V502_MOS_OP_STA_ZPG     = 0x85,
    V502_MOS_OP_STA_X_ZPG   = 0x95,
    V502_MOS_OP_STA_ABS     = 0x8D,
    V502_MOS_OP_STA_X_ABS   = 0x9D,
    V502_MOS_OP_STA_Y_ABS   = 0x99,

    V502_MOS_OP_PHA         = 0x48,
    V502_MOS_OP_PLA         = 0x68,

    V502_MOS_OP_LDA_NOW     = 0xA9,
    V502_MOS_OP_LDA_ZPG     = 0xA5,
    V502_MOS_OP_LDA_X_ZPG   = 0xB5,
    V502_MOS_OP_LDA_ABS     = 0xAD,
    V502_MOS_OP_LDA_X_ABS   = 0xBD,
    V502_MOS_OP_LDA_Y_ABS   = 0xB9,
    V502_MOS_OP_LDA_X_IND   = 0xA1,
    V502_MOS_OP_LDA_Y_IND   = 0xB1,

    V502_MOS_OP_ADC_NOW     = 0x69,
    V502_MOS_OP_ADC_ZPG     = 0x65,
    V502_MOS_OP_ADC_X_ZPG   = 0x75,
    V502_MOS_OP_ADC_ABS     = 0x6D,
    V502_MOS_OP_ADC_X_ABS   = 0x7D,
    V502_MOS_OP_ADC_Y_ABS   = 0x79,
    V502_MOS_OP_ADC_X_IND   = 0x61,
    V502_MOS_OP_ADC_Y_IND   = 0x71,

    V502_MOS_OP_AND_NOW     = 0x29,
    V502_MOS_OP_AND_ZPG     = 0x25,
    V502_MOS_OP_AND_X_ZPG   = 0x35,
    V502_MOS_OP_AND_ABS     = 0x2D,
    V502_MOS_OP_AND_X_ABS   = 0x3D,
    V502_MOS_OP_AND_Y_ABS   = 0x39,
    V502_MOS_OP_AND_X_IND   = 0x21,
    V502_MOS_OP_AND_Y_IND   = 0x31,

    V502_MOS_OP_SBC_NOW     = 0xE9,

    V502_MOS_OP_CMP_NOW     = 0xC9,

    //
    // State register
    //
    V502_MOS_OP_PHP         = 0x08,
    V502_MOS_OP_PLP         = 0x28,

    //
    // Flow
    //
    V502_MOS_OP_JMP_ABS     = 0x4C,
    V502_MOS_OP_JMP_IND     = 0x6C,

    V502_MOS_OP_JSR_ABS     = 0x20,
    V502_MOS_OP_RTS         = 0x60,

    //
    // Branching
    //
    V502_MOS_OP_BPL         = 0x10,
    V502_MOS_OP_BMI         = 0x30,
    V502_MOS_OP_BVC         = 0x50,
    V502_MOS_OP_BVS         = 0x70,
    V502_MOS_OP_BCC         = 0x90,
    V502_MOS_OP_BCS         = 0xB0,
    V502_MOS_OP_BNE         = 0xD0,
    V502_MOS_OP_BEQ         = 0xF0,

    //
    // Misc
    //
    V502_MOS_OP_NOP         = 0x1A // Not a real instruction, wastes a cycle though!
} V502_MOS_OP_E;

typedef enum V502_OP_STATE {
    V502_OP_STATE_FAILED, // Tells the VM something went wrong
    V502_OP_STATE_SUCCESS, // Tells the VM we passed
    V502_OP_STATE_SUCCESS_NO_COUNT // Tells the VM we passed but don't want to increment the program counter
} V502_OP_STATE_E;

//
// C <-> VM delegate
//
typedef struct v502_6502vm v502_6502vm_t;

typedef V502_OP_STATE_E(*v502_opfunc_t)(v502_6502vm_t*, v502_byte_t);

// Optional helper for defining opfuncs faster
#define V502_DEFINE_OPFUNC(NAME) V502_OP_STATE_E OP_##NAME(v502_6502vm_t* vm, v502_byte_t op)
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
