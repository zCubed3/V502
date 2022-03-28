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

    if (op == v502_MOS_OP_ADC_X_IND || op == v502_MOS_OP_ADC_Y_IND) {
        where = v502_make_word(vm->hunk[vm->program_counter + 2], vm->hunk[vm->program_counter + 1]);
        where += (op == v502_MOS_OP_ADC_X_IND ? vm->index_x : 0);

        where = v502_make_word(vm->hunk[where + 1], vm->hunk[where]) + (op == v502_MOS_OP_ADC_Y_IND ? vm->index_y : 0);
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

//
// X Register
//

v502_DEFINE_OPFUNC(INX) {
    vm->index_x += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(DEX) {
    vm->index_x -= 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(TSX) {
    vm->index_x = vm->stack_ptr;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(TXS) {
    vm->stack_ptr = vm->index_x;
    return V502_OP_STATE_SUCCESS;
}

//
// Y Register
//

v502_DEFINE_OPFUNC(INY) {
    vm->index_y += 1;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(DEY) {
    vm->index_y -= 1;
    return V502_OP_STATE_SUCCESS;
}

//
// Stack ops
//

v502_DEFINE_OPFUNC(PLA) {
    vm->accumulator = vm->hunk[v502_make_word(0x01, ++vm->stack_ptr)];
    vm->hunk[v502_make_word(0x01, vm->stack_ptr)] = 0;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(PHA) {
    vm->hunk[v502_make_word(0x01, vm->stack_ptr--)] = vm->accumulator;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(PLP) {
    vm->flags = vm->hunk[v502_make_word(0x01, ++vm->stack_ptr)];
    vm->hunk[v502_make_word(0x01, vm->stack_ptr)] = 0;
    return V502_OP_STATE_SUCCESS;
}

v502_DEFINE_OPFUNC(PHP) {
    vm->hunk[v502_make_word(0x01, vm->stack_ptr--)] = vm->flags;
    return V502_OP_STATE_SUCCESS;
}

//
// Combined ops
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

// ComPare Register
v502_DEFINE_OPFUNC(CPR) {
    v502_byte_t lhs = vm->accumulator;
    v502_byte_t rhs = 0;

    if (op == v502_MOS_OP_CPX_NOW)
        lhs = vm->index_x;

    if (op == v502_MOS_OP_CMP_NOW || op == v502_MOS_OP_CPX_NOW) {
        rhs = vm->hunk[++vm->program_counter];
    }

    v502_compare_vm(vm, lhs, rhs);
    return V502_OP_STATE_SUCCESS;
}

// LoaD Register
v502_DEFINE_OPFUNC(LDR) {
    uint8_t* what = &vm->accumulator;
    uint16_t where;

    if (op == v502_MOS_OP_LDX_NOW)
        what = &vm->index_x;

    if (op == v502_MOS_OP_LDA_NOW || op == v502_MOS_OP_LDX_NOW)
        where = ++vm->program_counter;

    if (op == v502_MOS_OP_LDA_ZPG)
        where = v502_make_word(0x00, ++vm->program_counter);

    *what = vm->hunk[where];

    return V502_OP_STATE_SUCCESS;
}


//
// Flow control ops
//

v502_DEFINE_OPFUNC(NOP) {
    return V502_OP_STATE_SUCCESS; // Waste a cycle
}

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

v502_DEFINE_OPFUNC(JSR) {
    vm->hunk[v502_make_word(0x01, vm->stack_ptr--)] = (vm->program_counter + 2);
    vm->hunk[v502_make_word(0x01, vm->stack_ptr--)] = (vm->program_counter + 2) >> 8;
    vm->program_counter = v502_make_word(vm->hunk[vm->program_counter + 2], vm->hunk[vm->program_counter + 1]);

    return V502_OP_STATE_SUCCESS_NO_COUNT;
}

v502_DEFINE_OPFUNC(RTS) {
    v502_byte_t l = vm->hunk[v502_make_word(0x01, vm->stack_ptr + 2)];
    v502_byte_t h = vm->hunk[v502_make_word(0x01, vm->stack_ptr + 1)];
    vm->program_counter = v502_make_word(h, l);

    vm->hunk[v502_make_word(0x01, ++vm->stack_ptr)] = 0;
    vm->hunk[v502_make_word(0x01, ++vm->stack_ptr)] = 0;

    return V502_OP_STATE_SUCCESS;
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

    vm->opfuncs[v502_MOS_OP_LDA_NOW] = OP_LDR;
    vm->opfuncs[v502_MOS_OP_LDA_ZPG] = OP_LDR;

    vm->opfuncs[v502_MOS_OP_CMP_NOW] = OP_CPR;

    //
    // X Register
    //
    vm->opfuncs[v502_MOS_OP_INX] = OP_INX;
    vm->opfuncs[v502_MOS_OP_DEX] = OP_DEX;

    vm->opfuncs[v502_MOS_OP_TXS] = OP_TXS;
    vm->opfuncs[v502_MOS_OP_TSX] = OP_TSX;

    vm->opfuncs[v502_MOS_OP_LDX_NOW] = OP_LDR;

    vm->opfuncs[v502_MOS_OP_CPX_NOW] = OP_CPR;

    //
    // Y Register
    //
    vm->opfuncs[v502_MOS_OP_INY] = OP_INY;
    vm->opfuncs[v502_MOS_OP_DEY] = OP_DEY;

    //
    // Stack ops
    //
    vm->opfuncs[v502_MOS_OP_PLA] = OP_PLA;
    vm->opfuncs[v502_MOS_OP_PHA] = OP_PHA;

    vm->opfuncs[v502_MOS_OP_PLP] = OP_PLP;
    vm->opfuncs[v502_MOS_OP_PHP] = OP_PHP;

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
    vm->opfuncs[v502_MOS_OP_NOP] = OP_NOP;

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

    vm->opfuncs[v502_MOS_OP_JSR_ABS] = OP_JSR;

    vm->opfuncs[v502_MOS_OP_RTS] = OP_RTS;

    // TODO: W65C02 Ops
    if (vm->feature_set == v502_FEATURESET_W65C02) {

    }

    for (int o = 0; o < 256; o++)
        if (vm->opfuncs[o] == NULL)
            vm->opfuncs[o] = OP_UNKNOWN;
}

v502_opfunc_t v502_get_fallback_func() {
    return OP_UNKNOWN;
}