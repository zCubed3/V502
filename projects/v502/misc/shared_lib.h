#ifndef V502_SHARED_LIB_H
#define V502_SHARED_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../vm/6502_ops.h"
#include "../vm/6502_vm.h"

#ifdef V502_INCLUDE_ASSEMBLER
#include "../assembler/assembler_symbol.h"
#include "../assembler/assembler.h"
#endif

typedef struct v502_function_table {
    v502_6502vm_t*(*v502_create_vm)(v502_6502vm_createinfo_t*);
    void(*v502_reset_vm)(v502_6502vm_t*);
    int(*v502_cycle_vm)(v502_6502vm_t*);

    v502_word_t(*v502_make_word)(v502_byte_t, v502_byte_t);

#ifdef V502_INCLUDE_ASSEMBLER
    v502_assembler_instance_t*(*v502_create_assembler)();
    v502_binary_file_t*(*v502_assemble_source)(v502_assembler_instance_t*, const char*);
    const char*(*v502_disassemble_binary)(v502_assembler_instance_t*, v502_binary_file_t*, v502_disassembly_options_t*);

    int(*v502_symbol_has_opcode)(v502_assembler_symbol_t*, v502_byte_t);
    int(*v502_symbol_get_arg_width)(v502_assembler_symbol_t*, v502_byte_t);
    int(*v502_symbol_is_arg_address)(v502_assembler_symbol_t*, v502_byte_t);
    int(*v502_symbol_is_arg_indirect)(v502_assembler_symbol_t*, v502_byte_t);
    int(*v502_symbol_get_indexing)(v502_assembler_symbol_t*, v502_byte_t);
#endif

} v502_function_table_t;

#ifdef WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

EXPORT v502_function_table_t* v502_get_function_table();

#ifdef __cplusplus
}
#endif

#endif