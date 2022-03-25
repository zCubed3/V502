#ifndef V502_6502_VM_H
#define V502_6502_VM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../v502_types.h"
#include "6502_ops.h"

typedef enum v502_FEATURESET {
    v502_FEATURESET_MOS6502 = 0,
    v502_FEATURESET_W65C02 = 1
} v502_FEATURESET_E;

typedef enum v502_STATE_FLAGS {
    v502_STATE_FLAG_CARRY       = 1,
    v502_STATE_FLAG_ZERO        = 2,
    v502_STATE_FLAG_INTERRUPT   = 4,
    v502_STATE_FLAG_DECIMAL     = 8,
    v502_STATE_FLAG_BREAK       = 16,
    // Bit 5 has no purpose
    v502_STATE_FLAG_OVERFLOW    = 64,
    v502_STATE_FLAG_NEGATIVE    = 128
} v502_STATE_FLAGS_E;


//
// Creation helper
//
typedef struct v502_6502vm_createinfo {
    v502_dword_t hunk_size;
    v502_FEATURESET_E feature_set;
} v502_6502vm_createinfo_t;

//
// The VM
//

// Magic start vector index, this was used by the actual 6502 for finding the origin of a program
extern const v502_word_t v502_MAGIC_VECTOR_INDEX;

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
    v502_FEATURESET_E feature_set;
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

// Adds and sets the overflow flag, please do this rather than add directly into the accumulator inside of C!
void v502_safe_add_vm(v502_6502vm_t *vm, v502_byte_t val);

// Subtracts and sets the overflow flag, please do this rather than subtracting directly from the accumulator inside of C!
void v502_safe_sub_vm(v502_6502vm_t *vm, v502_byte_t val);

// Sets the cpu flags accordingly for compare ops
void v502_compare_vm(v502_6502vm_t* vm, v502_byte_t lhs, v502_byte_t rhs);

#ifdef __cplusplus
};
#endif

#endif
