#include "assembler.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "../vm/6502_vm.h"

//
// Assembler
//

typedef struct source_line {
    char* line;
    uint32_t line_no;
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
    LABEL_REFERENCE_TYPE_RIGHT,
    LABEL_REFERENCE_TYPE_BRANCH
} LABEL_REFERENCE_TYPE_E;

typedef struct label_referencer {
    uint32_t where;
    uint32_t line_no;
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

void push_label_reference(label_placeholder_t* label, uint32_t where, LABEL_REFERENCE_TYPE_E type, uint32_t line_no) {
    assert(label != NULL);

    label_referencer_t* tail = NULL;
    for (label_referencer_t* child = label->top_ref; child != NULL; child = child->next) {
        tail = child;
    }

    label_referencer_t* ref = calloc(1, sizeof(label_referencer_t));
    ref->ref_type = type;
    ref->where = where;
    ref->line_no = line_no;

    if (tail == NULL)
        label->top_ref = ref;
    else
        tail->next = ref;
}

typedef enum MESSAGE_SEVERITY {
    MESSAGE_SEVERITY_NONE, // Special, this is for a top level message
    MESSAGE_SEVERITY_NOTE,
    MESSAGE_SEVERITY_WARNING,
    MESSAGE_SEVERITY_ERROR,
} MESSAGE_SEVERITY_E;

typedef struct message {
    const char* contents;
    uint32_t where;
    MESSAGE_SEVERITY_E severity;
    struct message* next;
} message_t;

void push_message(message_t* message, const char* contents, uint32_t where, MESSAGE_SEVERITY_E severity) {
    assert(message != NULL);

    message_t* tail = NULL;
    for (message_t* child = message->next; child != NULL; child = child->next) {
        tail = child;
    }

    message_t* msg = calloc(1, sizeof(message_t));
    msg->contents = contents;
    msg->severity = severity;
    msg->where = where;

    if (tail == NULL)
        message->next = msg;
    else
        tail->next = msg;
}

v502_assembler_instance_t* v502_create_assembler() {
    v502_assembler_instance_t *inst = calloc(1, sizeof(v502_assembler_instance_t));

    v502_symbol_setup_stack(&inst->symbol_stack);

    return inst;
}

const char* v502_load_source(const char* path) {
    FILE* file = fopen(path, "r");

    assert(file != NULL);

    fseek(file, 0, SEEK_END);
    uint32_t len = ftell(file);

    fseek(file, 0, SEEK_SET);

    char* source_str = malloc(len + 1); // + 1 to leave a trailing \0
    fread(source_str, len, 1, file);
    source_str[len] = '\0';

    fclose(file);

    return source_str;
}

v502_binary_file_t* v502_assemble_source(v502_assembler_instance_t* assembler, const char* source) {
    assert(assembler != NULL);
    assert(source != NULL);

    uint32_t source_len = strlen(source);

    char* source_dupe = calloc(source_len, 1);
    memcpy(source_dupe, source, source_len);

    source_line_stack_t* line_stack = calloc(1, sizeof(source_line_stack_t));
    label_placeholder_t* label_stack = NULL;
    message_t* message_stack = calloc(1, sizeof(message_t));

    // We find duplicate \n's and convert the first to a '\r'
    // This is to get around strtok() treating "\n\n" as one!
    uint32_t nl_count = 0;
    for (uint32_t c = 0; c < source_len; c++) {
        if (source_dupe[c] == '\n')
            nl_count++;
        else
            nl_count = 0;

        if (nl_count > 1) {
            source_dupe[c] = '\r';
            nl_count = 0;
        }
    }

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

        while (token[0] == '\r') {
            line_no += 1;
            token = token + 1;
        }

        // Trim extra '\r'
        while (token[0] == '\r')
            token = token + 1;

        // If this line starts with a period, it's assembler data
        if (token[0] == '.') {
            if (strstr(token, "org")) {
                if (origin_provided)
                    fprintf(stderr, "Multiple .org directives found, this is allowed but will override the previous directive!\n");

                uint32_t offset = 4;
                for (uint32_t s = 0; s < tok_len; s++) {
                    if (token[s] == '$') {
                        offset = s + 1;
                        break;
                    }
                }

                origin = strtol(token + offset, NULL, 16);

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
            fprintf(stderr, "Warning: On line %i there is a stray colon, did you mean to define a label?\n  src: '%s'\n", line_no, token);
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

        // Post sanitization check
        if (strlen(token) == 0)
            continue;

        char* line_dupe = malloc(strlen(token));
        strcpy(line_dupe, token);

        source_line_t* line = calloc(1, sizeof(source_line_t));
        line->line = line_dupe;
        line->line_no = line_no;

        push_source_line_stack(line_stack, line);
    }

    if (origin_provided)
        fprintf(stderr, "Explicit origin was provided, 0x%x\n", origin);
    else
        fprintf(stderr, "Origin wasn't provided, using default of 0x4000\n");

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
            if (child_label->line_def < child->line_no && !child_label->resolved) {
                child_label->loc = write_origin;
                child_label->resolved = 1;

                // TODO: If verbose
                fprintf(stderr, "Resolved label '%s' at 0x%x\n", child_label->symbol, child_label->loc);
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
        int open_parenthesis = 0;
        for (uint32_t c = 0; c < line_len; c++) {
            if (child->line[c] == '(')
                open_parenthesis = 1;

            if (child->line[c] == ')' && open_parenthesis) {
                open_parenthesis = 0;
                call_flags |= v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDIRECT;
            }
        }

        if (open_parenthesis) {
            push_message(message_stack, "Parenthesis left open!", child->line_no, MESSAGE_SEVERITY_ERROR);
            continue;
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

            if (child->line[c] == '$') {
                // Wide arg here is determined by how long the argument is!
                char *arg_text = child->line + c + 1;

                uint32_t end_idx = 0;
                for (uint32_t d = c; d < line_len; d++) {
                    if (child->line[d] != ',' && child->line[d] != ')')
                        end_idx = d;
                    else
                        break;
                }

                uint32_t len = end_idx - c;

                arg = strtol(arg_text, NULL, 16);
                wide_arg = len > 2;

                if (!wide_arg)
                    call_flags |= v502_ASSEMBLER_SYMBOL_CALL_FLAG_ZPG;

                // Check how this is indexed if it is
                for (uint32_t i = c; i < line_len; i++) {
                    if (child->line[i] == 'X') {
                        call_flags |= v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_X;
                        break;
                    }

                    if (child->line[i] == 'Y') {
                        call_flags |= v502_ASSEMBLER_SYMBOL_CALL_FLAG_INDEX_Y;
                        break;
                    }
                }

                has_arg = 1;
                break;
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
                                    indexer = child->line[b + 1] - '0';
                                }

                                if (child->line[b] == ']') {
                                    open_brackets = 0;
                                    has_indexer = 1;
                                }
                            }

                            assert(!open_brackets);

                            LABEL_REFERENCE_TYPE_E ref_type = LABEL_REFERENCE_TYPE_WHOLE;
                            if (has_indexer) {
                                if (indexer == 0)
                                    ref_type = LABEL_REFERENCE_TYPE_LEFT;
                                else if (indexer == 1)
                                    ref_type = LABEL_REFERENCE_TYPE_RIGHT;
                                else {
                                    push_message(message_stack, "Label indexer out of range, only 0 and 1 can be used!", child->line_no, MESSAGE_SEVERITY_ERROR);
                                }
                            }

                            if (sym->flags & v502_ASSEMBLER_SYMBOL_FLAG_RELATIVE)
                                ref_type = LABEL_REFERENCE_TYPE_BRANCH;

                            push_label_reference(child_label, write_origin + 1, ref_type, child->line_no);

                            has_arg = 1;
                            wide_arg = !has_indexer;

                            if (ref_type == LABEL_REFERENCE_TYPE_BRANCH)
                                wide_arg = 0;

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

        if (opcode == v502_ASSEMBLER_MAGIC_MISSING_CODE) {
            push_message(message_stack, "Missing opcode for given operation, is there a syntax error?", child->line_no, MESSAGE_SEVERITY_ERROR);
            continue;
        }

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
            fprintf(stderr, "Label '%s' was unresolved after assembling!\n", label->symbol);
            push_message(message_stack, "Label wasn't resolved!", label->line_def, MESSAGE_SEVERITY_ERROR);
        }

        for (label_referencer_t *ref = label->top_ref; ref != NULL; ref = ref->next) {
            if (ref->ref_type != LABEL_REFERENCE_TYPE_BRANCH) {
                if (ref->ref_type == LABEL_REFERENCE_TYPE_WHOLE || ref->ref_type == LABEL_REFERENCE_TYPE_LEFT)
                    binary_hunk[ref->where] = (char) label->loc;

                if (ref->ref_type == LABEL_REFERENCE_TYPE_WHOLE || ref->ref_type == LABEL_REFERENCE_TYPE_RIGHT)
                    binary_hunk[ref->where + (ref->ref_type == LABEL_REFERENCE_TYPE_WHOLE ? 1 : 0)] = (char) (label->loc >> 8);
            } else {
                uint16_t start = label->loc;
                uint16_t end = ref->where;
                int16_t rel = end - start;

                if (rel < -127 || rel > 128) {
                    fprintf(stderr, "ERROR: Long branch detected on line %i!\n", ref->line_no);
                    fprintf(stderr, "  Attempt to jump %i spaces! You can only move 127 bytes back and 128 forward!", ref->line_no);
                }

                binary_hunk[ref->where] = (char)(start - end);
            }
        }
    }

    int has_error = 0;
    for (message_t* message = message_stack; message != NULL; message = message->next) {
        if (message->severity == MESSAGE_SEVERITY_ERROR)
            has_error = 1;

        if (message->severity == MESSAGE_SEVERITY_NONE || message->contents == NULL)
            continue;

        switch (message->severity) {
            default:
                break;

            case MESSAGE_SEVERITY_NOTE:
                fprintf(stderr, "NOTE: ");
                break;

            case MESSAGE_SEVERITY_WARNING:
                fprintf(stderr, "WARNING: ");
                break;

            case MESSAGE_SEVERITY_ERROR:
                fprintf(stderr, "ERROR: ");
                break;
        }

        fprintf(stderr, "%s\n", message->contents);
        fprintf(stderr, "  on line %i\n", message->where);
    }

    v502_binary_file_t* bin_file = calloc(1, sizeof(v502_binary_file_t));
    bin_file->bytes = binary_hunk;
    bin_file->length = 0xFFFF + 1;

    return bin_file;
}

//
// Disassembly
//
#define DASM_TEMP_PATH "v502_dasm.temp"

const char* v502_disassemble_binary(v502_assembler_instance_t* assembler, v502_binary_file_t* file) {
    FILE* temp_file = fopen(DASM_TEMP_PATH, "wb"); // Binary to prevent windows from writing in UTF-16

    // First locate the origin point
    uint32_t origin = v502_make_word(file->bytes[v502_MAGIC_VECTOR_INDEX + 1], file->bytes[v502_MAGIC_VECTOR_INDEX]);

    fputs("; Generated by v502_disassemble_binary()!\n", temp_file);
    fputs("; This is a disassembled version of a binary file, elements such as .word, .byte, and other assembler traits will be missing!\n\n", temp_file);
    fprintf(temp_file, ".org $%x\n\n", origin);

    // Then start ripping out instructions
    uint32_t read_origin = origin;
    int reading = 1;

    while (reading) {
        v502_byte_t op = file->bytes[read_origin++];

        v502_assembler_symbol_t *sym = NULL;
        for (v502_assembler_symbol_t *child = assembler->symbol_stack; child != NULL; child = child->next) {
            if (v502_symbol_has_opcode(child, op)) {
                sym = child;
                break;
            }
        }

        if (sym == NULL)
            break;

        int width = v502_symbol_get_arg_width(sym, op);
        v502_word_t arg = 0;

        if (width != 0) {
            if (width == 1)
                arg = v502_make_word(0x00, file->bytes[read_origin++]);
            else {
                arg = v502_make_word(file->bytes[read_origin + 1], file->bytes[read_origin]);
                read_origin += 2;
            }
        }

        fputs(sym->name, temp_file);
        int is_addr = v502_symbol_is_arg_address(sym, op);
        int indirect = v502_symbol_is_arg_indirect(sym, op);
        int indexing = v502_symbol_get_indexing(sym, op);

        if (width != 0) {
            fputc(' ', temp_file);

            if (indirect)
                fputc('(', temp_file);

            if (!is_addr)
                fputc('#', temp_file);

            fputc('$', temp_file);

            if (width == 1)
                fprintf(temp_file, "%02x", arg);
            else
                fprintf(temp_file, "%04x", arg);

            if (indexing != 0) {
                if (indirect && indexing == 2)
                    fputc(')', temp_file);

                fprintf(temp_file, ",%c", (indexing == 1 ? 'X' : 'Y'));

                if (indirect && indexing == 1)
                    fputc(')', temp_file);
            } else
                if (indirect)
                    fputc(')', temp_file);
        }

        fprintf(temp_file, " ; %02x ", op);

        if (width > 0) {
            fprintf(temp_file, "%02x ", (char)arg);

            if (width > 1)
                fprintf(temp_file, "%02x ", (char)(arg >> 8));
        }

        fprintf(temp_file, "\n");
    }

    fclose(temp_file);

    // Read back from the buffer
    temp_file = fopen(DASM_TEMP_PATH, "rb");

    fseek(temp_file, 0, SEEK_END);
    uint32_t file_len = ftell(temp_file);

    fseek(temp_file, 0, SEEK_SET);
    char* buf = calloc(file_len + 1, 1);

    fread(buf, file_len, 1, temp_file);

    fclose(temp_file);
    remove(DASM_TEMP_PATH);

    return buf;
}