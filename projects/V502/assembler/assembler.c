#include "assembler.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "../vm/6502_vm.h"

typedef struct source_line {
    char* line;
    uint32_t no;
    struct source_line* next;
} source_line_t;

typedef struct source_line_stack {
    source_line_t* top;
    source_line_t* end;
} source_line_stack_t;

void push_source_line_stack(source_line_stack_t* stack, source_line_t* line) {
    assert(stack != NULL);

    for (source_line_t* child = stack->top; child != NULL; child = child->next) {
        stack->end = child;
    }

    if (stack->top == NULL && stack->end == NULL)
        stack->top = stack->end = line;
    else
        stack->end = stack->end->next = line;
}

typedef enum LABEL_REFERENCE_TYPE {
    LABEL_REFERENCE_TYPE_WHOLE,
    LABEL_REFERENCE_TYPE_LEFT,
    LABEL_REFERENCE_TYPE_RIGHT
} LABEL_REFERENCE_TYPE_E;

typedef struct label_referencer {
    uint32_t where;
    LABEL_REFERENCE_TYPE_E ref_type;
    struct label_referencer* next;
} label_referencer_t;

typedef struct label_placeholder {
    char* symbol;
    uint8_t resolved;
    uint32_t loc;
    uint32_t line_def;

    label_referencer_t* top_ref;
    struct label_placeholder* next;
} label_placeholder_t;

void push_label_placeholder(label_placeholder_t* stack, label_placeholder_t* label) {
    assert(stack != NULL);

    label_placeholder_t* tail = NULL;
    for (label_placeholder_t* child = stack; child != NULL; child = child->next) {
        tail = child;
    }

    assert(tail != NULL);

    tail->next = label;
}

void push_label_reference(label_placeholder_t* label, uint32_t where, LABEL_REFERENCE_TYPE_E type) {
    assert(label != NULL);

    label_referencer_t* tail = NULL;
    for (label_referencer_t* child = label->top_ref; child != NULL; child = child->next) {
        tail = child;
    }

    label_referencer_t* ref = calloc(1, sizeof(label_referencer_t));
    ref->ref_type = type;
    ref->where = where;

    if (tail == NULL)
        label->top_ref = ref;
    else
        tail->next = ref;
}

v502_assembler_instance_t* v502_create_assembler() {
    v502_assembler_instance_t *inst = calloc(1, sizeof(v502_assembler_instance_t));

    v502_symbol_setup_stack(&inst->symbol_stack);

    return inst;
}

v502_source_file_t* v502_load_source(const char* path) {
    FILE* file = fopen(path, "r");

    assert(file != NULL);

    fseek(file, 0, SEEK_END);
    uint32_t len = ftell(file);

    fseek(file, 0, SEEK_SET);

    char* source_str = malloc(len + 1); // + 1 to leave a trailing \0
    fread(source_str, len, 1, file);
    source_str[len] = '\0';

    fclose(file);

    v502_source_file_t* source = calloc(1, sizeof(v502_source_file_t));
    source->source = source_str;

    return source;
}

v502_binary_file_t* v502_assemble_source(v502_assembler_instance_t* assembler, v502_source_file_t* source) {
    assert(assembler != NULL);
    assert(source != NULL);

    uint32_t lines = 0;
    uint32_t source_len = strlen(source->source);

    char* source_dupe = malloc(source_len);
    memcpy(source_dupe, source->source, source_len);

    //printf("source:\n%s\n", source->source);

    source_line_stack_t* line_stack = calloc(1, sizeof(source_line_stack_t));
    label_placeholder_t* label_stack = NULL;

    // Seek backward and set the last \n to nothing to prevent errors caused by LF endings
    uint32_t last_line_idx = source_len - 1;
    while (source_dupe[last_line_idx] != '\n')
        last_line_idx--;

    // We find duplicate \n's and convert the first to a '\r'
    // This is to get around strtok() treating "\n\n" as one!
    uint32_t nl_count = 0;
    for (uint32_t c = 0; c < source_len; c++) {
        if (source_dupe[c] == '\n')
            nl_count++;
        else
            nl_count = 0;

        if (nl_count > 1)
            source_dupe[c] = '\r';
    }

    source_dupe[last_line_idx] = '\0';

    // Compiler data
    v502_word_t origin = 0x4000;
    int origin_provided = 0;

    // Then break the file up by lines
    uint32_t line_no = 0;
    uint32_t tok_inited = 0;

    while (1) {
        char* token = strtok((tok_inited ? NULL : source_dupe), "\n");
        tok_inited = 1;
        line_no += 1;

        if (token == NULL)
            break;

        uint32_t tok_len = strlen(token);

        if (tok_len == 0)
            continue;

        // If this line starts with a comment completely discard it
        // or if it's a null character
        if (token[0] == ';' || token[0] == '\0')
            continue;

        if (token[0] == '\r') {
            line_no += 1;
            token = token + 1;
        }

        // If this line starts with a period, it's assembler data
        if (token[0] == '.') {
            if (strstr(token, "org")) {
                if (origin_provided)
                    printf("Multiple .org directives found, this is allowed but will override the previous directive!\n");

                sscanf(token, "org %hi", &origin);

                origin_provided = 1;
            }

            continue;
        }

        // We zero out the rest following a ';'
        int redacting = 0;
        for (uint32_t c = 0; c < tok_len; c++) {
            if (token[c] == ';')
                redacting = 1;

            if (redacting)
                token[c] = '\0';
        }

        // We also eliminate trailing characters
        for (uint32_t c = tok_len - 1; c > 0; c--) {
            if (token[c] == '\n' || token[c] == '\r' || token[c] == ' ' || token[c] == '\0')
                token[c] = '\0';
            else
                break;
        }

        // We also trim the start to remove tabs and spaces, and extra \r's from CRLF files
        uint32_t offset = 0;
        for (; offset < tok_len; offset++) {
            if (!(token[offset] == ' ' || token[offset] == '\t'))
                break;
        }

        // If this line starts with a colon, discard it since it's an empty label
        if (token[0] == ':') {
            printf("Warning: On line %i there is a stray colon, did you mean to define a label?\n  src: '%s'\n", line_no, token);
            continue;
        }

        // Check if this contains a colon at the end of a name, if so, we've found a label!
        uint32_t label_found = 0;
        for (uint32_t c = 0; c < tok_len; c++) {
            if (token[c] == ':') {
                char* label = calloc(c + 1, 1);
                memcpy(label, token, c);

                label_placeholder_t* placeholder = calloc(1, sizeof(label_placeholder_t));
                placeholder->symbol = label;
                placeholder->line_def = line_no;

                if (label_stack != NULL)
                    push_label_placeholder(label_stack, placeholder);
                else
                    label_stack = placeholder;

                label_found = 1;
                break;
            }
        }

        if (label_found)
            continue;

        token = token + offset;

        char* line_dupe = malloc(strlen(token));
        strcpy(line_dupe, token);

        source_line_t* line = calloc(1, sizeof(source_line_t));
        line->line = line_dupe;
        line->no = line_no;

        push_source_line_stack(line_stack, line);
    }

    if (origin_provided)
        printf("Explicit origin was provided, 0x%x\n", origin);
    else
        printf("Origin wasn't provided, using default of 0x4000\n");

    // Begin assembling
    char* binary_hunk = calloc(0xFFFF + 1, 1);

    // Write the origin
    binary_hunk[v502_MAGIC_VECTOR_INDEX] = (char)origin;
    binary_hunk[v502_MAGIC_VECTOR_INDEX + 1] = (char)(origin >> 8);

    // Parse the assembly lines
    uint32_t write_origin = origin;
    for (source_line_t* child = line_stack->top; child != NULL; child = child->next) {
        // If we have unresolved labels behind this line we must resolve them
        for (label_placeholder_t* child_label = label_stack; child_label != NULL; child_label = child_label->next) {
            if (child_label->line_def < child->no && !child_label->resolved) {
                child_label->loc = write_origin;
                child_label->resolved = 1;

                // TODO: If verbose
                printf("Resolved label '%s' at 0x%x\n", child_label->symbol, child_label->loc);
            }
        }

        uint32_t line_len = strlen(child->line);

        // The opcode is the first 3 characters
        char op[4];
        memcpy(op, child->line, 3);

        for (int c = 0; c < 3; c++)
            op[c] = (char) toupper(op[c]);

        // Locate the symbol
        v502_assembler_symbol_t *sym = NULL;
        for (v502_assembler_symbol_t *child = assembler->symbol_stack; child != NULL; child = child->next) {
            if (strcmp(op, child->name) == 0) {
                sym = child;
                break;
            }
        }

        assert(sym != NULL);

        // Then determine the type of call
        v502_ASSEMBLER_SYMBOL_CALL_FLAGS_E call_flags = 0;

        // If we encounter parenthesis, this is indirect!
        // We do need to close the parenthesis though!
        // TODO: Error stack!
        int open_parenthesis = 0;
        for (uint32_t c = 0; c < line_len; c++) {
            if (child->line[c] == '(')
                open_parenthesis = 1;

            if (child->line[c] == ')' && open_parenthesis) {
                open_parenthesis = 0;
                call_flags |= v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDIRECT;
            }
        }

        // If we encounter a number marker, we are doing a NOW call
        // or if we encounter a stray address without indirect!
        int wide_arg = 0, has_arg = 0;
        v502_word_t arg = 0;
        for (uint32_t c = 0; c < line_len; c++) {
            if (child->line[c] == '#') {
                char ident = child->line[c + 1];
                char *arg_text = child->line + c + 2;

                if (ident == '$')
                    arg = strtol(arg_text, NULL, 16);
                else if (ident == '%')
                    arg = strtol(arg_text, NULL, 2);
                else
                    arg = strtol(arg_text, NULL, 10);

                has_arg = 1;
                break;
            }

            if (!(call_flags & v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDIRECT)) {
                if (child->line[c] == '$') {
                    // Wide arg here is determined by how long the resulting string is!
                    char *arg_text = child->line + c + 1;
                    uint32_t len = strlen(arg_text);

                    arg = strtol(arg_text, NULL, 16);
                    wide_arg = len > 2;

                    if (!wide_arg)
                        call_flags |= v502_ASSEMBLER_SYMBOL_CALL_FLAG_ZPG;

                    has_arg = 1;
                    break;
                }
            }
        }

        // If we're missing an arg, check if we're referencing a label we found in preprocessing
        if (!has_arg) {
            for (uint32_t c = 4; c < line_len; c++) {
                if (child->line[c] != ' ') {
                    for (label_placeholder_t *child_label = label_stack; child_label != NULL; child_label = child_label->next) {
                        // We need to single out the label name
                        char* label_name = calloc(strlen(child_label->symbol), 1);

                        for (uint32_t b = c; b < line_len; b++) {
                            if (child->line[b] == '[')
                                break;
                            else
                                label_name[b - c] = child->line[b];
                        }

                        if (strcmp(label_name, child_label->symbol) == 0) {
                            // We first need to check if there are indexer brackets
                            int has_indexer = 0, open_brackets = 0, indexer = 0;
                            for (uint32_t b = c; b < line_len; b++) {
                                if (child->line[b] == '[') {
                                    open_brackets = 1;
                                    indexer = strtol(child->line + b, NULL, 10);
                                }

                                if (child->line[b] == ']') {
                                    open_brackets = 0;
                                    has_indexer = 1;
                                }
                            }

                            LABEL_REFERENCE_TYPE_E ref_type = LABEL_REFERENCE_TYPE_WHOLE;
                            if (has_indexer) {
                                if (indexer == 0)
                                    ref_type = LABEL_REFERENCE_TYPE_LEFT;
                                else if (indexer == 1)
                                    ref_type = LABEL_REFERENCE_TYPE_RIGHT;
                                else
                                    assert(ref_type); // TODO: Error stack
                            }

                            push_label_reference(child_label, write_origin + 1, ref_type);

                            has_arg = 1;
                            wide_arg = !has_indexer;
                            break;
                        }

                        free(label_name);
                    }
                    break;
                }
            }
        }

        // We then pass this into v502_symbol_get_opcode
        v502_word_t opcode = v502_symbol_get_opcode(sym, call_flags, wide_arg);

        // TODO: Error stack!
        assert(opcode != v502_ASSEMBLER_MAGIC_MISSING_CODE);

        binary_hunk[write_origin++] = (char) opcode;

        if (has_arg) {
            binary_hunk[write_origin++] = (char) arg;
            if (wide_arg)
                binary_hunk[write_origin++] = (char) (arg >> 8);
        }
    }

    // After compiling we have to go through and populate the label references
    for (label_placeholder_t *label = label_stack; label != NULL; label = label->next) {
        if (!label->resolved) {
            printf("Error: Label '%s' was unresolved after assembling!\n", label->symbol);
            printf("  '%s' was defined on line %i\n", label->symbol, label->line_def);

            // TODO: Error stack
            assert(label->resolved);
        }

        for (label_referencer_t *ref = label->top_ref; ref != NULL; ref = ref->next) {
            if (ref->ref_type == LABEL_REFERENCE_TYPE_WHOLE || ref->ref_type == LABEL_REFERENCE_TYPE_LEFT)
                binary_hunk[ref->where] = (char)label->loc;

            if (ref->ref_type == LABEL_REFERENCE_TYPE_WHOLE || ref->ref_type == LABEL_REFERENCE_TYPE_RIGHT)
                binary_hunk[ref->where + 1] = (char)(label->loc >> 8);
        }
    }


    v502_binary_file_t* bin_file = calloc(1, sizeof(v502_binary_file_t));
    bin_file->bytes = binary_hunk;
    bin_file->length = 0xFFFF + 1;

    return bin_file;
}