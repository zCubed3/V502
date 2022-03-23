#ifndef V502_ASSEMBLER_H
#define V502_ASSEMBLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "asm_symbol.h"
#include "../v502_types.h"

//
// Result structures
//
typedef struct v502_binary_file {
    char* bytes;
    uint32_t length;
} v502_binary_file_t;

//
// Assembler
//
typedef struct v502_assembler_instance {
    // Symbols are a linked list (stack)
    v502_assembler_symbol_t* symbol_stack;
} v502_assembler_instance_t;

v502_assembler_instance_t* v502_create_assembler();

const char* v502_load_source(const char* path);
v502_binary_file_t* v502_assemble_source(v502_assembler_instance_t* assembler, const char* source);

#ifdef __cplusplus
}
#endif

#endif
