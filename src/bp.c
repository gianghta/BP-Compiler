#include <stdlib.h>
#include <stdio.h>

#include "include/bp.h"
#include "include/io.h"
#include "include/lexer.h"
#include "include/token.h"
#include "include/parser.h"
#include "include/semantic.h"


void bp_compile(char* src, const char* name, bool parser_flag, bool table_flag, bool jit_flag)
{
    set_file_name(name);
    Semantic* sem = init_semantic_analyzer();
    lexer_T* lexer = init_lexer(src, sem);
    parser_T* parser = init_parser(lexer, sem, parser_flag, table_flag, jit_flag);

    if(output_bitcode(parser))
    {
        printf("Successfully generate code.\n");
    }

    // Cleanup
    free(lexer);
    free(parser);
    free(sem);
    lexer = NULL;
    parser = NULL;
    sem = NULL;
}

void bp_compile_file(const char* filename, bool parser_flag, bool table_flag, bool jit_flag)
{
    char* src = bp_read_file(filename);
    bp_compile(src, filename, parser_flag, table_flag, jit_flag);
    free(src);
}