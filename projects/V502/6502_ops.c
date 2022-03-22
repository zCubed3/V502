#include "6502_ops.h"
#include "6502_vm.h"

#include <stdio.h>

v502_DEFINE_OPFUNC(UNKNOWN) {
    printf("V502: Unknown instruction 0x%x\n", op);
    return V502_OP_STATE_FAILED;
}

v502_DEFINE_OPFUNC(ADC) {
    if (op == v502_MOS_OP_ADC_NOW)
        v502_safe_add_vm(vm, vm->hunk[++vm->program_counter]);

    if (op == v502_MOS_OP_ADC_ZPG || op == v502_MOS_OP_ADC_X_ZPG) {
        // Forced into a byte to wrap around
        v502_byte_t zpg = vm->hunk[++vm->program_counter] + (op == v502_MOS_OP_ADC_X_ZPG ? vm->index_x : 0);
        v502_safe_add_vm(vm, vm->hunk[zpg]);
    }

    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(STA) {
    if (op == v502_MOS_OP_STA_ZPG)
        vm->hunk[vm->hunk[++vm->program_counter]] = vm->accumulator;

    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(JMP) {
    if (op == v502_MOS_OP_JMP_ABS)
        vm->program_counter = v502_make_word(vm->hunk[vm->program_counter + 2], vm->hunk[vm->program_counter + 1]);

    return V502_OP_STATE_SUCCESS_NO_COUNT;
}

//
// Transfer ops
//

// Accumulator -> Register
v502_DEFINE_OPFUNC(TAR) {
    *(op == v502_MOS_OP_TAX ? &vm->index_x : &vm->index_y) = vm->accumulator;
    return V502_OP_STATE_SUCCESS;
}

// Register -> Accumulator
v502_DEFINE_OPFUNC(TRA) {
    vm->accumulator = (op == v502_MOS_OP_TXA ? vm->index_x : vm->index_y);
    return V502_OP_STATE_SUCCESS;
}

//
// opfunc array populate function
//
void v502_populate_ops_vm(v502_6502vm_t* vm) {
    vm->opfuncs[v502_MOS_OP_ADC_NOW] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_ZPG] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_X_ZPG] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_ABS] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_X_ABS] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_Y_ABS] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_X_IND] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_Y_IND] = OP_ADC;

    vm->opfuncs[v502_MOS_OP_STA_ZPG] = OP_STA;

    vm->opfuncs[v502_MOS_OP_JMP_ABS] = OP_JMP;

    vm->opfuncs[v502_MOS_OP_TAX] = OP_TAR;
    vm->opfuncs[v502_MOS_OP_TAY] = OP_TAR;

    vm->opfuncs[v502_MOS_OP_TXA] = OP_TRA;
    vm->opfuncs[v502_MOS_OP_TYA] = OP_TRA;

    // TODO: W65C02 Ops
    if (vm->feature_set == v502_FEATURESET_W65C02) {

    }

    for (int o = 0; o < 256; o++)
        if (vm->opfuncs[o] == NULL)
            vm->opfuncs[o] = OP_UNKNOWN;
}