#include <stdlib.h>
#include <stdio.h>

#include "include/bp.h"
#include "include/io.h"
#include "include/lexer.h"
#include "include/token.h"
#include "include/parser.h"


void bp_compile(char* src)
{
    lexer_T* lexer = init_lexer(src);
    // Token* token = lexer_get_next_token(lexer);


    // while (token->type != T_EOF)
    // {
    //     char* str = print_token(token);
    //     printf("%s", str);
    //     token = lexer_get_next_token(lexer);
    // }
    
    // char* str = print_token(token);
    // printf("%s\n", str);

    parser_T* parser = init_parser(lexer);
    printf("\nStart parsing....\n");
    bool status = parse(parser);

    printf("Parse result is: %s\n", status ? "true" : "false");
}

void bp_compile_file(const char* filename)
{
    char* src = bp_read_file(filename);
    bp_compile(src);
    free(src);
}