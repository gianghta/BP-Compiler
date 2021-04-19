#include <stdlib.h>
#include <stdio.h>

#include "include/bp.h"
#include "include/io.h"
#include "include/lexer.h"
#include "include/token.h"
#include "include/parser.h"
#include "include/semantic.h"


void bp_compile(char* src)
{
    Semantic* sem = init_semantic_analyzer();
    lexer_T* lexer = init_lexer(src, sem);
    parser_T* parser = init_parser(lexer, sem);

    outputAssembly(parser);

    // Cleanup
    free(lexer);
    free(parser);
    free(sem);
    lexer = NULL;
    parser = NULL;
    sem = NULL;
}

void bp_compile_file(const char* filename)
{
    char* src = bp_read_file(filename);
    bp_compile(src);
    free(src);
}