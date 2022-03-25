#include "6502_ops.h"
#include "6502_vm.h"

#include <stdio.h>

v502_DEFINE_OPFUNC(UNKNOWN) {
    printf("V502: Unknown instruction 0x%x\n", op);
    return V502_OP_STATE_FAILED;
}

//
// Accumulator functions
//

v502_DEFINE_OPFUNC(ADC) {
    uint16_t where;
    if (op == v502_MOS_OP_ADC_NOW)
        where = ++vm->program_counter;

    if (op == v502_MOS_OP_ADC_ZPG || op == v502_MOS_OP_ADC_X_ZPG)
        where = v502_make_word(0x00, vm->hunk[++vm->program_counter] + (op == v502_MOS_OP_ADC_X_ZPG ? vm->index_x : 0));

    if (op == v502_MOS_OP_ADC_ABS || op == v502_MOS_OP_ADC_X_ABS || op == v502_MOS_OP_ADC_Y_ABS) {
        where = v502_make_word(vm->hunk[vm->program_counter + 2], vm->hunk[vm->program_counter + 1]);
        where += (op == v502_MOS_OP_STA_ABS ? 0 : (op == v502_MOS_OP_STA_X_ABS ? vm->index_x : vm->index_y));
        vm->program_counter += 2;
    }

    v502_safe_add_vm(vm, vm->hunk[where]);
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(STA) {
    uint16_t where = 0;
    if (op == v502_MOS_OP_STA_ZPG || op == v502_MOS_OP_STA_X_ZPG)
        where = v502_make_word(0x00, vm->hunk[++vm->program_counter] + (op == v502_MOS_OP_STA_X_ZPG ? vm->index_x : 0));

    if (op == v502_MOS_OP_STA_ABS || op == v502_MOS_OP_STA_X_ABS || op == v502_MOS_OP_STA_Y_ABS) {
        where = v502_make_word(vm->hunk[vm->program_counter + 2], vm->hunk[vm->program_counter + 1]);
        where += (op == v502_MOS_OP_STA_ABS ? 0 : (op == v502_MOS_OP_STA_X_ABS ? vm->index_x : vm->index_y));
        vm->program_counter += 2;
    }

    vm->hunk[where] = vm->accumulator;

    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(LDA) {
    uint16_t where;
    if (op == v502_MOS_OP_LDA_NOW)
        where = ++vm->program_counter;

    vm->accumulator = vm->hunk[where];
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(CMP) {
    v502_byte_t rhs = 0;

    if (op == v502_MOS_OP_CMP_NOW)
        rhs = vm->hunk[++vm->program_counter];

    v502_compare_vm(vm, vm->accumulator, rhs);
    return V502_OP_STATE_SUCCESS;
}

//
// X Register
//

v502_DEFINE_OPFUNC(INX) {
    vm->index_x += 1;
    return V502_OP_STATE_SUCCESS;
}

//
// Y Register
//

v502_DEFINE_OPFUNC(INY) {
    vm->index_y += 1;
    return V502_OP_STATE_SUCCESS;
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
// Flow control ops
//

v502_DEFINE_OPFUNC(BPL) {
    if (!(vm->flags & v502_STATE_FLAG_NEGATIVE)) {
        vm->program_counter += (int8_t) vm->hunk[++vm->program_counter];
        return V502_OP_STATE_SUCCESS_NO_COUNT;
    }

    // If we don't branch, skip the argument
    vm->program_counter += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(BMI) {
    if (vm->flags & v502_STATE_FLAG_NEGATIVE) {
        vm->program_counter += (int8_t) vm->hunk[++vm->program_counter];
        return V502_OP_STATE_SUCCESS_NO_COUNT;
    }

    // If we don't branch, skip the argument
    vm->program_counter += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(BVC) {
    if (!(vm->flags & v502_STATE_FLAG_OVERFLOW)) {
        vm->program_counter += (int8_t) vm->hunk[++vm->program_counter];
        return V502_OP_STATE_SUCCESS_NO_COUNT;
    }

    // If we don't branch, skip the argument
    vm->program_counter += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(BVS) {
    if (vm->flags & v502_STATE_FLAG_OVERFLOW) {
        vm->program_counter += (int8_t) vm->hunk[++vm->program_counter];
        return V502_OP_STATE_SUCCESS_NO_COUNT;
    }

    // If we don't branch, skip the argument
    vm->program_counter += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(BCC) {
    if (!(vm->flags & v502_STATE_FLAG_CARRY)) {
        vm->program_counter += (int8_t) vm->hunk[++vm->program_counter];
        return V502_OP_STATE_SUCCESS_NO_COUNT;
    }

    // If we don't branch, skip the argument
    vm->program_counter += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(BCS) {
    if (vm->flags & v502_STATE_FLAG_CARRY) {
        vm->program_counter += (int8_t) vm->hunk[++vm->program_counter];
        return V502_OP_STATE_SUCCESS_NO_COUNT;
    }

    // If we don't branch, skip the argument
    vm->program_counter += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(BNE) {
    if (!(vm->flags & v502_STATE_FLAG_CARRY && vm->flags & v502_STATE_FLAG_ZERO)) {
        vm->program_counter += (int8_t) vm->hunk[++vm->program_counter];
        return V502_OP_STATE_SUCCESS_NO_COUNT;
    }

    // If we don't branch, skip the argument
    vm->program_counter += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(BEQ) {
    if (vm->flags & v502_STATE_FLAG_CARRY && vm->flags & v502_STATE_FLAG_ZERO) {
        vm->program_counter += (int8_t) vm->hunk[++vm->program_counter];
        return V502_OP_STATE_SUCCESS_NO_COUNT;
    }

    // If we don't branch, skip the argument
    vm->program_counter += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(JMP) {
    if (op == v502_MOS_OP_JMP_ABS)
        vm->program_counter = v502_make_word(vm->hunk[vm->program_counter + 2], vm->hunk[vm->program_counter + 1]);

    if (op == v502_MOS_OP_JMP_IND) {
        uint16_t ind = v502_make_word(vm->hunk[vm->program_counter + 2], vm->hunk[vm->program_counter + 1]);
        vm->program_counter = v502_make_word(vm->hunk[ind + 1], vm->hunk[ind]);
    }

    return V502_OP_STATE_SUCCESS_NO_COUNT;
}

//
// opfunc array populate function
//
void v502_populate_ops_vm(v502_6502vm_t* vm) {
    //
    // Accumulator ops
    //
    vm->opfuncs[v502_MOS_OP_ADC_NOW] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_ZPG] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_X_ZPG] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_ABS] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_X_ABS] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_Y_ABS] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_X_IND] = OP_ADC;
    vm->opfuncs[v502_MOS_OP_ADC_Y_IND] = OP_ADC;

    vm->opfuncs[v502_MOS_OP_STA_ZPG] = OP_STA;
    vm->opfuncs[v502_MOS_OP_STA_X_ZPG] = OP_STA;
    vm->opfuncs[v502_MOS_OP_STA_ABS] = OP_STA;
    vm->opfuncs[v502_MOS_OP_STA_X_ABS] = OP_STA;
    vm->opfuncs[v502_MOS_OP_STA_Y_ABS] = OP_STA;

    vm->opfuncs[v502_MOS_OP_LDA_NOW] = OP_LDA;

    vm->opfuncs[v502_MOS_OP_CMP_NOW] = OP_CMP;

    //
    // X Register
    //
    vm->opfuncs[v502_MOS_OP_INX] = OP_INX;

    //
    // Y Register
    //
    vm->opfuncs[v502_MOS_OP_INY] = OP_INY;

    //
    // Transfers
    //
    vm->opfuncs[v502_MOS_OP_TAX] = OP_TAR;
    vm->opfuncs[v502_MOS_OP_TAY] = OP_TAR;

    vm->opfuncs[v502_MOS_OP_TXA] = OP_TRA;
    vm->opfuncs[v502_MOS_OP_TYA] = OP_TRA;

    //
    // Branching / Flow
    //
    vm->opfuncs[v502_MOS_OP_BPL] = OP_BPL;
    vm->opfuncs[v502_MOS_OP_BMI] = OP_BMI;
    vm->opfuncs[v502_MOS_OP_BVC] = OP_BVC;
    vm->opfuncs[v502_MOS_OP_BVS] = OP_BVS;
    vm->opfuncs[v502_MOS_OP_BCC] = OP_BCC;
    vm->opfuncs[v502_MOS_OP_BCS] = OP_BCS;
    vm->opfuncs[v502_MOS_OP_BNE] = OP_BNE;
    vm->opfuncs[v502_MOS_OP_BEQ] = OP_BEQ;

    vm->opfuncs[v502_MOS_OP_JMP_ABS] = OP_JMP;
    vm->opfuncs[v502_MOS_OP_JMP_IND] = OP_JMP;

    // TODO: W65C02 Ops
    if (vm->feature_set == v502_FEATURESET_W65C02) {

    }

    for (int o = 0; o < 256; o++)
        if (vm->opfuncs[o] == NULL)
            vm->opfuncs[o] = OP_UNKNOWN;
}