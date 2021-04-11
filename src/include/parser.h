#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
#include "token.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct PARSER_STRUCT
{
    lexer_T* lexer;
    Token* current_token;
    Token* look_ahead;
} parser_T;

parser_T* init_parser(lexer_T* lexer);
bool parser_eat(parser_T* parser, TokenType type);

bool is_token_type(parser_T* parser, TokenType type);

bool parse(parser_T* parser);
bool program(parser_T* parser);
bool program_header(parser_T* parser);
bool program_body(parser_T* parser);
bool declaration(parser_T* parser);

bool procedure_declaration(parser_T* parser);
bool procedure_header(parser_T* parser);
bool parameter_list(parser_T* parser);
bool parameter(parser_T* parser);
bool procedure_body(parser_T* parser);

bool variable_declaration(parser_T* parser);
bool type_mark(parser_T* parser);
bool bound(parser_T* parser);

bool statement(parser_T* parser);
bool procedure_call(parser_T* parser);
bool assignment_statement(parser_T* parser);
bool destination(parser_T* parser);
bool if_statement(parser_T* parser);
bool loop_statement(parser_T* parser);
bool return_statement(parser_T* parser);

bool identifier(parser_T* parser);

bool expression(parser_T* parser);
bool expression_prime(parser_T* parser);
bool arith_op(parser_T* parser);
bool arith_op_prime(parser_T* parser);
bool relation(parser_T* parser);
bool relation_prime(parser_T* parser);
bool term(parser_T* parser);
bool term_prime(parser_T* parser);
bool factor(parser_T* parser);

bool procedure_call_or_name_handler(parser_T* parser);
bool name(parser_T* parser);
bool array_index(parser_T* parser);
bool argument_list(parser_T* parser);
bool number(parser_T* parser);
bool string(parser_T* parser);

bool declaration_list(parser_T* parser);
bool statement_list(parser_T* parser);

#endif