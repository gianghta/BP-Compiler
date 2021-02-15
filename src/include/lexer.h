#ifndef LEXER_H
#define LEXER_H
#include "token.h"

typedef struct LEXER_STRUCT
{
    char current_char;
    unsigned int start;
    unsigned int current;
    char* source;
} Lexer;

Lexer* init_lexer(char* contents);

void lexer_advance(Lexer* lexer);

// Methods to handle blankspace and comments
void lexer_skip_whitespace(Lexer* lexer);
void lexer_skip_line_comment(Lexer* lexer);
void lexer_skip_block_comment(Lexer* lexer);

Token* lexer_get_next_token(Lexer* lexer);

Token* lexer_collect_string(Lexer* lexer);

Token* lexer_collect_id(Lexer* lexer);

Token* lexer_collect_integer(Lexer* lexer);

Token* lexer_collect_float(Lexer** lexer, Token** token);

Token* lexer_collect_char(Lexer* lexer);

#endif