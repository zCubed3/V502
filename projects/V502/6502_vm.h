#ifndef V502_6502_VM_H
#define V502_6502_VM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "6502_types.h"
#include "6502_ops.h"

typedef enum V502_FEATURESET {
    V502_FEATURESET_MOS6502 = 0,
    V502_FEATURESET_W65C02 = 1
} V502_FEATURESET_E;


//
// Creation helper
//
typedef struct v502_6502vm_createinfo {
    v502_dword_t hunk_size;
    V502_FEATURESET_E feature_set;
} v502_6502vm_createinfo_t;

//
// The VM
//

// Magic start vector index, this was used by the actual 6502 for finding the origin of a program
extern const v502_word_t V502_MAGIC_VECTOR_INDEX;

typedef struct v502_6502vm {
    v502_word_t program_counter;

    v502_byte_t stack_ptr;
    v502_byte_t accumulator;
    v502_byte_t index_x;
    v502_byte_t index_y;
    v502_byte_t flags;

    v502_byte_t *hunk;
    v502_dword_t hunk_length;

    v502_opfunc_t* opfuncs;
    V502_FEATURESET_E feature_set;
} v502_6502vm_t;

//
// Actual behavior
//

// NOTE: Creating the VM doesn't initialize it, call v502_reset_vm() to initialize it!
v502_6502vm_t *v502_create_vm(v502_6502vm_createinfo_t *p_createinfo);

void v502_reset_vm(v502_6502vm_t *vm);

int v502_cycle_vm(v502_6502vm_t *vm);

//
// Helpers
//
v502_word_t v502_make_word(v502_byte_t a, v502_byte_t b);

#ifdef __cplusplus
};
#endif

#endif
