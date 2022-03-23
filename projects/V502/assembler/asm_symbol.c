#include "asm_symbol.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

extern const v502_word_t v502_ASSEMBLER_MAGIC_MISSING_CODE = 0xFFFF;

#define OP(NAME) v502_MOS_OP_##NAME

#define MISSING v502_ASSEMBLER_MAGIC_MISSING_CODE
#define FLAG_NONE 0
#define FLAG_WIDE v502_ASSEMBLER_SYMBOL_FLAG_INDIRECT_WORD
#define FLAG_REL v502_ASSEMBLER_SYMBOL_FLAG_RELATIVE
#define FLAG_BOTH (v502_ASSEMBLER_SYMBOL_FLAG_INDIRECT_WORD | v502_ASSEMBLER_SYMBOL_FLAG_RELATIVE)

#include "../vm/6502_ops.h"

// This is clunky to be completely honest, but it's the best option we have!
// This is better than pushing everything by hand
const v502_assembler_symbol_t SYMBOL_TABLE[] = {
    // { "", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, FLAG_ },

    //
    // Stack operations
    //
    { "PHA", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(PHA), FLAG_NONE, NULL },
    { "PHP", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(PHP), FLAG_NONE, NULL },
    { "PLA", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(PLA), FLAG_NONE, NULL },
    { "PLP", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(PLP), FLAG_NONE, NULL },

    //
    // X Register
    //
    { "INX", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(INX), FLAG_NONE, NULL },
    { "DEX", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(DEX), FLAG_NONE, NULL },
    { "TAX", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(TAX), FLAG_NONE, NULL },
    { "TXA", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(TXA), FLAG_NONE, NULL },
    { "TSX", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(TSX), FLAG_NONE, NULL },
    { "TXS", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(TXS), FLAG_NONE, NULL },
    { "LDX", OP(LDX_ZPG), MISSING, OP(LDX_Y_ZPG), OP(LDX_ABS), MISSING, OP(LDX_Y_ABS), MISSING, MISSING, MISSING, OP(LDX_NOW), MISSING, FLAG_NONE, NULL },
    { "CPX", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(CPX_NOW), MISSING, FLAG_NONE, NULL },
    { "STX", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, FLAG_NONE, NULL },

    //
    // Y Register
    //
    { "INY", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(INY), FLAG_NONE, NULL },
    { "DEY", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(DEY), FLAG_NONE, NULL },
    { "TAY", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(TAY), FLAG_NONE, NULL },
    { "TYA", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(TYA), FLAG_NONE, NULL },
    { "LDY", OP(LDY_ZPG), OP(LDY_X_ZPG), MISSING, OP(LDY_ABS), OP(LDY_X_ABS), MISSING, MISSING, MISSING, MISSING, OP(LDY_NOW), MISSING, FLAG_NONE, NULL },
    { "CPY", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(LDY_NOW), MISSING, FLAG_NONE, NULL },
    { "STY", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, FLAG_NONE, NULL },

    //
    // A Register
    //
    { "STA", OP(STA_ZPG), OP(STA_X_ZPG), MISSING, OP(STA_ABS), OP(STA_X_ABS), OP(STA_Y_ABS), MISSING, MISSING, MISSING, MISSING, MISSING, FLAG_NONE, NULL },
    { "LDA", OP(LDA_ZPG), OP(LDA_X_ZPG), MISSING, OP(LDA_ABS), OP(LDA_X_ABS), OP(LDA_Y_ABS), MISSING, OP(LDA_X_IND), OP(LDA_Y_IND), OP(LDA_NOW), MISSING, FLAG_NONE, NULL },
    { "ADC", OP(ADC_ZPG), OP(ADC_X_ZPG), MISSING, OP(ADC_ABS), OP(ADC_X_ABS), OP(ADC_Y_ABS), MISSING, OP(ADC_X_IND), OP(ADC_Y_IND), OP(ADC_NOW), MISSING, FLAG_NONE, NULL },
    { "SBC", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(SBC_NOW), MISSING, FLAG_NONE, NULL },
    { "CMP", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(CMP_NOW), MISSING, FLAG_NONE, NULL },
    { "AND", OP(AND_ZPG), OP(AND_X_ZPG), MISSING, OP(AND_ABS), OP(AND_X_ABS), OP(AND_Y_ABS), MISSING, OP(AND_X_IND), OP(AND_Y_IND), OP(AND_NOW), MISSING, FLAG_NONE, NULL },

    //
    // Flow
    //
    { "JMP", MISSING, MISSING, MISSING, OP(JMP_ABS), MISSING, MISSING, OP(JMP_IND), MISSING, MISSING, MISSING, MISSING, FLAG_WIDE, NULL },
    { "JSR", MISSING, MISSING, MISSING, OP(JSR_ABS), MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, FLAG_WIDE, NULL },
    { "RTS", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(RTS), FLAG_NONE, NULL },

    //
    // Branching
    //
    { "BPL", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(BPL), FLAG_REL, NULL },
    { "BMI", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(BMI), FLAG_REL, NULL },
    { "BVC", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(BVC), FLAG_REL, NULL },
    { "BVS", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(BVS), FLAG_REL, NULL },
    { "BCC", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(BCC), FLAG_REL, NULL },
    { "BCS", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(BCS), FLAG_REL, NULL },
    { "BNE", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(BNE), FLAG_REL, NULL },
    { "BEQ", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(BEQ), FLAG_REL, NULL },

    //
    // Misc
    //
    { "NOP", MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, MISSING, OP(NOP), FLAG_NONE, NULL },

    // Special end marker element
    { "TABLE_END" },
};

v502_assembler_symbol_t* v502_symbol_create() {
    v502_assembler_symbol_t* sym = calloc(1, sizeof(v502_assembler_symbol_t));

    sym->zpg = sym->x_zpg = sym->y_zpg =
    sym->abs = sym->x_abs = sym->y_abs =
    sym->ind = sym->x_ind = sym->y_ind =
    sym->now = sym->only = v502_ASSEMBLER_MAGIC_MISSING_CODE;

    sym->next = NULL;

    return sym;
}

void v502_symbol_push_stack(v502_assembler_symbol_t* top, v502_assembler_symbol_t* new) {
    assert(top != NULL);
    assert(new != NULL);

    v502_assembler_symbol_t* empty = NULL;

    for (v502_assembler_symbol_t* child = top; child != NULL; child = child->next) {
        empty = child;
    }

    assert(empty != NULL);

    empty->next = new;
}

void v502_symbol_setup_stack(v502_assembler_symbol_t** top) {
    assert(top != NULL);

    if (*top == NULL)
        *top = v502_symbol_create();

    **top = SYMBOL_TABLE[0];

    for (int s = 1; ; s++) {
        v502_assembler_symbol_t sym = SYMBOL_TABLE[s];

        if (strcmp(sym.name, "TABLE_END") == 0)
            break;

        v502_assembler_symbol_t* tail_sym = v502_symbol_create();
        *tail_sym = sym;

        v502_symbol_push_stack(*top, tail_sym);
    }
}

v502_word_t v502_symbol_get_opcode(v502_assembler_symbol_t* sym, v502_ASSEMBLER_SYMBOL_CALL_FLAGS_E call_flags, int wide_arg) {
    assert(sym != NULL);

    if (sym->only != MISSING)
        return sym->only;

    if (wide_arg) {
        if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDIRECT && sym->flags & v502_ASSEMBLER_SYMBOL_FLAG_INDIRECT_WORD) {
            if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_X)
                return sym->x_ind;

            if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_Y)
                return sym->y_ind;

            return sym->ind;
        } else {
            if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_X)
                return sym->x_abs;

            if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_Y)
                return sym->y_abs;

            return sym->abs;
        }
    } else {
        if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDIRECT) {
            if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_X)
                return sym->x_ind;

            if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_Y)
                return sym->y_ind;

            return sym->ind;
        } else {
            if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_ZPG) {
                if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_X)
                    return sym->x_zpg;

                if (call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_Y)
                    return sym->y_zpg;

                return sym->zpg;
            }

            return sym->now;
        }
    }
}