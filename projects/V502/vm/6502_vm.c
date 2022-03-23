#include "6502_vm.h"

#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

extern const v502_word_t v502_MAGIC_VECTOR_INDEX = 0xFFFC;

v502_word_t v502_make_word(v502_byte_t a, v502_byte_t b) {
    return (a << 8) | b;
}

v502_6502vm_t* v502_create_vm(v502_6502vm_createinfo_t* p_createinfo) {
    assert(p_createinfo != NULL);

    v502_6502vm_t* vm = calloc(1, sizeof(v502_6502vm_t));

    vm->hunk_length = p_createinfo->hunk_size;
    vm->hunk = calloc(1, vm->hunk_length);

    vm->feature_set = p_createinfo->feature_set;

    vm->opfuncs = calloc(256, sizeof(v502_opfunc_t));
    v502_populate_ops_vm(vm);

    return vm;
}

void v502_reset_vm(v502_6502vm_t *vm) {
    assert(vm != NULL);

    v502_word_t org = v502_make_word(vm->hunk[v502_MAGIC_VECTOR_INDEX + 1], vm->hunk[v502_MAGIC_VECTOR_INDEX]);

    vm->program_counter = org;
}

int v502_cycle_vm(v502_6502vm_t* vm) {
    assert(vm != NULL);

    v502_byte_t next_op = vm->hunk[vm->program_counter];
    v502_OP_STATE_E state = vm->opfuncs[next_op](vm, next_op);

    if (state <= 0)
        return 0;

    if (state != V502_OP_STATE_SUCCESS_NO_COUNT)
        vm->program_counter += 1;

    return 1;
}

void v502_safe_add_vm(v502_6502vm_t *vm, v502_byte_t val) {
    v502_word_t r = vm->accumulator + val;
    r += vm->flags & v502_STATE_FLAG_CARRY ? 1 : 0; // Adds one to r if we've got a carry bit

    if (r < 0x00 || r > 0xFF)
        vm->flags |= (v502_STATE_FLAG_OVERFLOW | v502_STATE_FLAG_CARRY);
    else
        vm->flags &= ~(v502_STATE_FLAG_OVERFLOW | v502_STATE_FLAG_CARRY);

    vm->accumulator += val;
}

//TODO: I haven't tested if this actually lines up with SDC behavior on an actual 6502
void v502_safe_sub_vm(v502_6502vm_t *vm, v502_byte_t val) {
    v502_word_t r = vm->accumulator + -(int8_t)val;
    r += vm->flags & v502_STATE_FLAG_CARRY ? 1 : 0; // Adds one to r if we've got a carry bit

    if (r < 0x00 || r > 0xFF)
        vm->flags |= (v502_STATE_FLAG_OVERFLOW | v502_STATE_FLAG_CARRY);
    else
        vm->flags &= ~(v502_STATE_FLAG_OVERFLOW | v502_STATE_FLAG_CARRY);

    vm->accumulator += -(int8_t)val;
}