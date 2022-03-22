#include "6502_ops.h"
#include "6502_vm.h"

#include <stdio.h>

V502_DEFINE_OPFUNC(UNKNOWN) {
    printf("V502: Unknown instruction 0x%x\n", op);
    return V502_OP_STATE_FAILED;
}

V502_DEFINE_OPFUNC(ADC) {
    if (op == V502_MOS_OP_ADC_NOW)
        vm->accumulator += vm->hunk[++vm->program_counter];

    return V502_OP_STATE_SUCCESS;
}

V502_DEFINE_OPFUNC(STA) {
    if (op == V502_MOS_OP_STA_ZPG)
        vm->hunk[vm->hunk[++vm->program_counter]] = vm->accumulator;

    return V502_OP_STATE_SUCCESS;
}

V502_DEFINE_OPFUNC(JMP) {
    if (op == V502_MOS_OP_JMP_ABS)
        vm->program_counter = v502_make_word(vm->hunk[vm->program_counter + 2], vm->hunk[vm->program_counter + 1]);

    return V502_OP_STATE_SUCCESS_NO_COUNT;
}

void v502_populate_ops_vm(v502_6502vm_t* vm) {
    vm->opfuncs[V502_MOS_OP_ADC_NOW] = OP_ADC;
    vm->opfuncs[V502_MOS_OP_STA_ZPG] = OP_STA;
    vm->opfuncs[V502_MOS_OP_JMP_ABS] = OP_JMP;

    // TODO: W65C02 Ops
    if (vm->feature_set == V502_FEATURESET_W65C02) {

    }

    for (int o = 0; o < 256; o++)
        if (vm->opfuncs[o] == NULL)
            vm->opfuncs[o] = OP_UNKNOWN;
}