#include "shared_lib.h"

#include <stdlib.h>

v502_function_table_t* v502_get_function_table() {
    v502_function_table_t* ftable = calloc(1, sizeof(v502_function_table_t));

    ftable->v502_create_vm = v502_create_vm;
    ftable->v502_reset_vm = v502_reset_vm;
    ftable->v502_cycle_vm = v502_cycle_vm;
    ftable->v502_get_fallback_func = v502_get_fallback_func;

    ftable->v502_make_word = v502_make_word;

#ifdef V502_INCLUDE_ASSEMBLER
    ftable->v502_create_assembler = v502_create_assembler;
    ftable->v502_assemble_source = v502_assemble_source;
    ftable->v502_disassemble_binary = v502_disassemble_binary;

    ftable->v502_symbol_has_opcode = v502_symbol_has_opcode;
    ftable->v502_symbol_get_arg_width = v502_symbol_get_arg_width;
    ftable->v502_symbol_is_arg_address = v502_symbol_is_arg_address;
    ftable->v502_symbol_is_arg_indirect = v502_symbol_is_arg_indirect;
    ftable->v502_symbol_get_indexing = v502_symbol_get_indexing;
#endif

    return ftable;
}