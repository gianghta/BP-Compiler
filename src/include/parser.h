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
void parser_eat(parser_T* parser, token_type type);
void parse(parser_T* parser);

void parse_program(parser_T* parser);
void parse_program_body(parser_T* parser);
void parse_declarations(parser_T* parser);
void parse_proc_declaration(parser_T* parser);
void parse_var_declaration(parser_T* parser);
void parse_type_mark(parser_T* parser);
void parse_statements(parser_T* parser);
void parse_statement_chain(parser_T* parser);
void parse_statement(parser_T* parser);
void parse_param(parser_T* parser);
void parse_param_list(parser_T* parser);
void parse_param_list_param(parser_T* parser);
void parse_assignment_statement(parser_T* parser);
void parse_if_statement(parser_T* parser);
void parse_loop_statement(parser_T* parser);
void parse_return_statement(parser_T* parser);
void parse_procedure_call(parser_T* parser);
void parse_destination(parser_T* parser);
void parse_argument_list(parser_T* parser);
void parse_argument_list_expression(parser_T* parser);
void parse_expression(parser_T* parser);
void parse_expression_arith_op(parser_T* parser);
void parse_arith_op(parser_T* parser);
void parse_arith_op_relation(parser_T* parser);
void parse_relation(parser_T* parser);
void parse_relation_term(parser_T* parser);
void parse_term(parser_T* parser);
void parse_term_factor(parser_T* parser);
void parse_factor(parser_T* parser);
void parse_indexes(parser_T* parser);

#endif