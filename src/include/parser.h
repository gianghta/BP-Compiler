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
bool parser_eat(parser_T* parser, token_type type);

bool is_token_type(parser_T* parser, token_type type);

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
bool arith_op_prime(parser_T parser);
bool relation(parser_T* parser);
bool relation_prime(parser_T* parser);
bool term(parser_T* parser);
bool term_prime(parser_T* parser);
bool factor(parser_T* parser);

bool declaration_list(parser_T* parser);
bool statement_list(parser_T* parser);

bool array_index(parser_T* parser);

// void parse(parser_T* parser);
// void parse_program(parser_T* parser);
// void parse_program_body(parser_T* parser);
// void parse_declarations(parser_T* parser);
// void parse_proc_declaration(parser_T* parser);
// void parse_var_declaration(parser_T* parser);
// void parse_type_mark(parser_T* parser);
// void parse_statements(parser_T* parser);
// void parse_statement(parser_T* parser);
// void parse_statement_list(parser_T* parser);
// void parse_param(parser_T* parser);
// void parse_param_list(parser_T* parser);
// void parse_param_list_param(parser_T* parser);
// void parse_assignment_statement(parser_T* parser);
// void parse_if_statement(parser_T* parser);
// void parse_loop_statement(parser_T* parser);
// void parse_return_statement(parser_T* parser);
// void parse_procedure_call(parser_T* parser);
// void parse_destination(parser_T* parser);
// void parse_argument_list(parser_T* parser);
// void parse_argument_list_expression(parser_T* parser);
// void parse_expression(parser_T* parser);
// void parse_expression_arith_op(parser_T* parser);
// void parse_arith_op(parser_T* parser);
// void parse_arith_op_relation(parser_T* parser);
// void parse_relation(parser_T* parser);
// void parse_relation_term(parser_T* parser);
// void parse_term(parser_T* parser);
// void parse_term_factor(parser_T* parser);
// void parse_factor(parser_T* parser);
// void parse_indexes(parser_T* parser);

#endif