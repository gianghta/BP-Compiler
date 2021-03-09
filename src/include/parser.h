#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
#include "token.h"

typedef struct PARSER_STRUCT
{
    lexer_T* lexer;
    Token* current_token;
    Token* look_ahead;
} parser_T;

parser_T* init_parser(lexer_T* lexer);
void match(Token* token);

void parse_program();
void parse_program_body();
void parse_declarations();
void parse_declaration();
void parse_proc_declaration();
void parse_var_declaration();

#endif