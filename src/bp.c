#include "include/bp.h"
#include "include/io.h"
#include "include/lexer.h"
#include <stdlib.h>


void bp_compile(char* src)
{
    lexer_T* lexer = init_lexer(src);
    Token* token = lexer_get_next_token(lexer);

    while (token->type != T_EOF)
    {
        print_token(token);
        token = lexer_get_next_token(lexer);
    }
}

void bp_compile_file(const char* filename)
{
    char* src = bp_read_file(filename);
    bp_compile(src);
    free(src);
}