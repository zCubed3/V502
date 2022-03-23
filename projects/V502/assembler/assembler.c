#include "assembler.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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
        for (uint32_t c = tok_len - 1; c > 0; c++) {
            if (token[c] == '\n' || token[c] == '\r' || token[c] == ' ' || token[c] == '\0')
                token[c] = '\0';
            else
                break;
        }

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

    // Actually assemble the lines
    for (source_line_t* child = line_stack->top; child != NULL; child = child->next) {
        char* line_dupe = malloc(strlen(child->line));
        strcpy(line_dupe, child->line);

        printf("%i: %s\n", child->no, child->line);
    }

    v502_binary_file_t* bin_file = calloc(1, sizeof(v502_binary_file_t));
    bin_file->bytes = malloc(128); // TODO

    return bin_file;
}