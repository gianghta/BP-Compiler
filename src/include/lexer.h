#ifndef LEXER_H
#define LEXER_H
#include "token.h"

typedef struct LEXER_STRUCT
{
    char current_char;
    unsigned int start;
    unsigned int current;
    char* source;
} lexer_T;

lexer_T* init_lexer(char* contents);

void lexer_advance(lexer_T* lexer);

// Methods to handle blankspace and comments
void lexer_skip_whitespace(lexer_T* lexer);
void lexer_skip_line_comment(lexer_T* lexer);
void lexer_skip_block_comment(lexer_T* lexer);

Token* lexer_get_next_token(lexer_T* lexer);
Token* lexer_collect_string(lexer_T* lexer);
Token* lexer_collect_id(lexer_T* lexer);
Token* lexer_collect_integer(lexer_T* lexer);
Token* lexer_collect_float(lexer_T** lexer, Token** token);

#endif