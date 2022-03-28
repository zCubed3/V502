#ifndef V502_ASSEMBLER_SYMBOL_H
#define V502_ASSEMBLER_SYMBOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../v502_types.h"

typedef enum v502_ASSEMBLER_SYMBOL_FLAGS {
    v502_ASSEMBLER_SYMBOL_FLAG_INDIRECT_WORD = 1,
    v502_ASSEMBLER_SYMBOL_FLAG_RELATIVE = 2
} v502_ASSEMBLER_SYMBOL_FLAGS_E;

typedef enum v502_ASSEMBLER_SYMBOL_CALL_FLAGS {
    v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDIRECT = 1,
    v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_X = 2,
    v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_Y = 4,
    v502_ASSEMBLER_SYMBOL_CALL_FLAG_ZPG = 8,
} v502_ASSEMBLER_SYMBOL_CALL_FLAGS_E;

#define v502_ASSEMBLER_MAGIC_MISSING_CODE 0xFFFF

typedef struct v502_assembler_symbol {
    const char* name;

    v502_word_t zpg, x_zpg, y_zpg;
    v502_word_t abs, x_abs, y_abs;
    v502_word_t ind, x_ind, y_ind;
    v502_word_t now, only; // If only is not set to v502_ASSEMBLER_MAGIC_MISSING_CODE
    v502_ASSEMBLER_SYMBOL_FLAGS_E flags; // if ind_word = true, indirect calls use words instead of bytes

    struct v502_assembler_symbol* next;
} v502_assembler_symbol_t;

void v502_symbol_setup_stack(v502_assembler_symbol_t** top);
v502_word_t v502_symbol_get_opcode(v502_assembler_symbol_t* sym, v502_ASSEMBLER_SYMBOL_CALL_FLAGS_E call_flags, int wide_arg);

int v502_symbol_has_opcode(v502_assembler_symbol_t* sym, v502_byte_t opcode);
int v502_symbol_get_arg_width(v502_assembler_symbol_t* sym, v502_byte_t opcode);
int v502_symbol_is_arg_address(v502_assembler_symbol_t* sym, v502_byte_t opcode);
int v502_symbol_is_arg_indirect(v502_assembler_symbol_t* sym, v502_byte_t opcode);
int v502_symbol_get_indexing(v502_assembler_symbol_t* sym, v502_byte_t opcode);

#ifdef __cplusplus
}
#endif

#endif
