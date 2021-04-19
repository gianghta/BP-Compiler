#ifndef PARSER_H
#define PARSER_H
#include "lexer.h"
#include "token.h"
#include "semantic.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>


typedef struct PARSER_STRUCT
{
    lexer_T* lexer;
    Semantic* sem;
    Token* current_token;
    Token* look_ahead;
} parser_T;

parser_T* init_parser(lexer_T* lexer, Semantic* sem);
bool parser_eat(parser_T* parser, TokenType type);

bool is_token_type(parser_T* parser, TokenType type);

bool parse(parser_T* parser);
bool program(parser_T* parser);
bool program_header(parser_T* parser);
bool program_body(parser_T* parser);
bool declaration(parser_T* parser);

bool procedure_declaration(parser_T* parser, Symbol* decl);
bool procedure_header(parser_T* parser, Symbol* decl);
bool parameter_list(parser_T* parser, Symbol* decl);
bool parameter(parser_T* parser, Symbol* param);
bool procedure_body(parser_T* parser);

bool variable_declaration(parser_T* parser, Symbol* decl);
bool type_mark(parser_T* parser, Symbol* id);
bool bound(parser_T* parser, Symbol* id);

bool statement(parser_T* parser);
bool procedure_call(parser_T* parser);
bool assignment_statement(parser_T* parser);
bool destination(parser_T* parser, Symbol* id);
bool if_statement(parser_T* parser);
bool loop_statement(parser_T* parser);
bool return_statement(parser_T* parser);

bool identifier(parser_T* parser, Symbol* id);

bool expression(parser_T* parser, Symbol* exp);
bool expression_prime(parser_T* parser, Symbol* exp);
bool arith_op(parser_T* parser, Symbol* op);
bool arith_op_prime(parser_T* parser, Symbol* op);
bool relation(parser_T* parser, Symbol* rel);
bool relation_prime(parser_T* parser, Symbol* rel);
bool term(parser_T* parser, Symbol* tm);
bool term_prime(parser_T* parser, Symbol* tm);
bool factor(parser_T* parser, Symbol* fac);

bool procedure_call_or_name_handler(parser_T* parser, Symbol* id);
bool name(parser_T* parser, Symbol* id);
bool array_index(parser_T* parser, Symbol* id);
bool argument_list(parser_T* parser, Symbol* id);
bool number(parser_T* parser, Symbol* num);
bool string(parser_T* parser, Symbol* str);

bool declaration_list(parser_T* parser);
bool statement_list(parser_T* parser);

bool type_checking(Symbol* dest, Symbol* exp);
bool expression_type_checking(Symbol* lhs, Symbol* rhs);
bool arithmetic_type_checking(Symbol* lhs, Symbol* rhs);
bool relation_type_checking(Symbol* lhs, Symbol* rhs, Token* op);

bool outputAssembly(parser_T* parser);
void array_assignment_codegen(parser_T* parser, Symbol* dest, Symbol* exp);

#endif