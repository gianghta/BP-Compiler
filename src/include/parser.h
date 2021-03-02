#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
#include "token.h"

typedef struct PARSER_STRUCT
{
    lexer_T* lexer;
    Token current_token;
    Token look_ahead;
} parser_T;

#endif