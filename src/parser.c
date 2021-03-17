#include "include/parser.h"
#include <stdlib.h>
#include <stdio.h>

parser_T* init_parser(lexer_T* lexer)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->current_token = (void*) 0;
    parser->look_ahead = lexer_get_next_token(lexer);
    return parser;
}

void parser_eat(parser_T* parser, token_type type)
{
    Token* tmp = init_token(type);
    printf("Matching token. Expected type: ");
    printf("%s\n", print_token(tmp));

    if (parser->look_ahead->type != type)
    {
        printf("Token doesn't match. Current look ahead is: %s\n", print_token(parser->look_ahead));
    }
    else
    {
        printf("Token matched. Current look ahead is: %s\n", print_token(parser->look_ahead));
        parser->current_token = parser->look_ahead;
        parser->look_ahead = lexer_get_next_token(parser->lexer);
    }
}

// Entry points
void parse(parser_T* parser)
{
    parse_program(parser);
}

void parse_program(parser_T* parser)
{
    printf("Parsing program\n");

    // program header
    parser_eat(parser, K_PROGRAM);
    parser_eat(parser, T_ID);
    parser_eat(parser, K_IS);

    parse_program_body(parser);

    if (parser->look_ahead->type == T_EOF) parser_eat(parser, T_EOF);

    printf("Program finished. Parsing finished\n");
}

void parse_program_body(parser_T* parser)
{
    // program body
    if (parser->look_ahead->type != K_BEGIN)
    {
        parse_declarations(parser);
    }

    parser_eat(parser, K_BEGIN);

    if (parser->look_ahead->type != K_END)
    {
        parse_statements(parser);
    }

    parser_eat(parser, K_END);
    parser_eat(parser, K_PROGRAM);
}

void parse_declarations(parser_T* parser)
{
    printf("Parsing declarations\n");
    switch (parser->look_ahead->type)
    {
    case K_GLOBAL:
        parser_eat(parser, K_GLOBAL);
        parse_declarations(parser);
        break;
    case K_PROCEDURE:
        parse_proc_declaration(parser);
        parser_eat(parser, T_SEMI_COLON);
        parse_declarations(parser);
        break;
    // Break out. Going back to program body to parse statements
    case K_BEGIN:
        break;
    default:
        parse_var_declaration(parser);
        parser_eat(parser, T_SEMI_COLON);
        parse_declarations(parser);
        break;
    }
    printf("Finished parsing declarations");
}

void parse_proc_declaration(parser_T* parser)
{
    printf("Parsing procedure\n");

    // procedure header
    parser_eat(parser, K_PROCEDURE);
    parser_eat(parser, T_ID);
    parser_eat(parser, T_COLON);
    parse_type_mark(parser);
    parser_eat(parser, T_LPAREN);
    if (parser->look_ahead->type != T_RPAREN)
    {
        parse_param_list(parser);
    }
    parser_eat(parser, T_RPAREN);

    // procedure body
    if (parser->look_ahead->type != K_BEGIN)
    {
        parse_declarations(parser);
    }
    parser_eat(parser, K_BEGIN);

    if (parser->look_ahead->type != K_END)
    {
        parse_statements(parser);
    }

    parser_eat(parser, K_END);
    parser_eat(parser, K_PROCEDURE);
    printf("Finished parsing procedure\n");
}

void parse_var_declaration(parser_T* parser)
{
    printf("Parsing variable.\n");
    parser_eat(parser, K_VARIABLE);
    parser_eat(parser, T_ID);
    parser_eat(parser, T_COLON);
    parse_type_mark(parser);

    // parsing array variable
    if (parser->look_ahead->type == T_LBRACKET)
    {
        parser_eat(parser, T_LBRACKET);
        parser_eat(parser, T_NUMBER_INT);
        parser_eat(parser, T_RBRACKET);
    }
    printf("Finished parsing variable\n");
}

void parse_type_mark(parser_T* parser)
{
    printf("Parsing type mark\n");
    switch (parser->look_ahead->type)
    {
        case K_INT:
            parser_eat(parser, K_INT); break;
        case K_FLOAT:
            parser_eat(parser, K_FLOAT); break;
        case K_BOOL:
            parser_eat(parser, K_BOOL); break;
        case K_STRING:
            parser_eat(parser, K_STRING); break;
        default:
            printf("Error. Invalid type\n");
            break;
    }
    printf("Finished parsing type mark.\n");
}

void parse_statements(parser_T* parser)
{
    parse_statement(parser);
    parse_statement_list(parser);
}

void parse_statement_list(parser_T* parser)
{
    switch(parser->look_ahead->type)
    {
        case T_SEMI_COLON:
            parser_eat(parser, T_SEMI_COLON);
            parse_statement(parser);
            parse_statement_list(parser);
            break;
        // Follow the end of if statement or end of statement
        case K_END:
        case K_ELSE:
            break;
        default:
            printf("Error. Invalid statement parsing.\n");
            break;
    }
}

void parse_statement(parser_T* parser)
{
    printf("Parsing statement.");
    switch (parser->look_ahead->type)
    {
        case K_IF: parse_if_statement(parser); break;
        case K_FOR: parse_loop_statement(parser); break;
        case K_RETURN: parse_return_statement(parser); break;
        case T_ID:
            parse_assignment_statement(parser);
            break;
        
        // Fallback if statement, for loop
        case K_END:
        case K_ELSE:
        case T_SEMI_COLON:
            break;
        default:
            printf("Error. Invalid statements.\n");
            break;
    }
    printf("Finish parsing statement.\n");
}

void parse_assignment_statement(parser_T* parser)
{
    printf("Parsing assignment statement.\n");
    parse_destination(parser);
    parser_eat(parser, T_ASSIGNMENT);
    parse_expression(parser);
    printf("Finished parsing assignment statement.\n");
}

void parse_if_statement(parser_T* parser)
{
    printf("Parsing if statement.\n");
    parser_eat(parser, K_IF);
    parser_eat(parser, T_LPAREN);
    parse_expression(parser);
    parser_eat(parser, T_RPAREN);
    parser_eat(parser, K_THEN);
    parse_statements(parser);
    if (parser->look_ahead->type == K_ELSE)
    {
        parser_eat(parser, K_ELSE);
        parse_statements(parser);
    }
    parser_eat(parser, K_END);
    parser_eat(parser, K_IF);
    printf("Finished parsing if statement\n");
}

void parse_loop_statement(parser_T* parser)
{
    printf("Parsing for loop statement.\n");
    parser_eat(parser, K_FOR);
    parser_eat(parser, T_LPAREN);
    parse_assignment_statement(parser);
    parser_eat(parser, T_SEMI_COLON);
    parse_expression(parser);
    parser_eat(parser, T_RPAREN);
    if (parser->look_ahead->type != K_END)
    {
        parse_statements(parser);
    }
    parser_eat(parser, K_END);
    parser_eat(parser, K_FOR);
    printf("Finished parsing for loop statement.\n");
}

void parse_return_statement(parser_T* parser)
{
    parser_eat(parser, K_RETURN);
    parse_expression(parser);
}
void parse_procedure_call(parser_T* parser)
{
    printf("Parsing procedure call.\n");
    parser_eat(parser, T_ID);
    parser_eat(parser, T_LPAREN);
    parse_argument_list(parser);
    parser_eat(parser, T_RPAREN);
    printf("Finished parsing procedure call.\n");
}

void parse_destination(parser_T* parser)
{
    printf("Parsing destination.\n");
    parser_eat(parser, T_ID);
    if (parser->look_ahead->type == T_LPAREN)
    {
        printf("This is not a destination. Backtrack to procedure call.\n");
        return;
    }

    parse_indexes(parser);

    printf("Finished parsing destination.\n");
}

void parse_argument_list(parser_T* parser)
{
    printf("Parsing an argument list.\n");
    parse_expression(parser);
    parse_argument_list_expression(parser);
    printf("Finished parsing argument list.\n");
}

void parse_argument_list_expression(parser_T* parser)
{
    switch (parser->look_ahead->type)
    {
        case T_COMMA:
            parser_eat(parser, T_COMMA);
            parse_expression(parser);
            parse_argument_list_expression(parser);
            break;
        case T_LPAREN:
            break;
        default:
            printf("Error. Invalid argument list parsing.\n");
            break;
    }
}

void parse_expression(parser_T* parser)
{
    printf("Parsing expression.\n");
    if (parser->look_ahead->type == K_NOT)
    {
        parser_eat(parser, K_NOT);
    }
    parse_arith_op(parser);
    parse_expression_arith_op(parser);
    printf("Finished parsing expressions.\n");
}

void parse_expression_arith_op(parser_T* parser)
{
    switch(parser->look_ahead->type)
    {
        case T_AND:
            parser_eat(parser, T_AND);
            parse_arith_op(parser);
            parse_expression_arith_op(parser);
            break;
        case T_OR:
            parser_eat(parser, T_OR);
            parse_arith_op(parser);
            parse_expression_arith_op(parser);
            break;
        case T_COMMA: // argument list
        case T_RPAREN: // for loop, if statement
        case T_RBRACKET: // assignment statement
        case T_SEMI_COLON: // statements
            break;
        default:
            printf("Error. Invalid expression parsing.\n");
            break;
    }
}

void parse_arith_op(parser_T* parser)
{
    printf("Parsing arithmetic operation.\n");
    parse_relation(parser);
    parse_arith_op_relation(parser);
    printf("Finished parsing arithmetic operation.\n");
}

void parse_arith_op_relation(parser_T* parser)
{
    switch(parser->look_ahead->type)
    {
        case T_PLUS:
            parser_eat(parser, T_PLUS);
            parse_relation(parser);
            parse_arith_op_relation(parser);
            break;
        case T_MINUS:
            parser_eat(parser, T_MINUS);
            parse_relation(parser);
            parse_arith_op_relation(parser);
            break;
        case T_AND: case T_OR: case T_COMMA: // expression
        case T_RPAREN: // for loop, if statement
        case T_RBRACKET: // assignment statement
        case T_SEMI_COLON:
            break;
        default:
            printf("Error. Invalid parsing arithmetic operator.\n");
            break;
    }
}

void parse_relation(parser_T* parser)
{
    printf("Parsing a relation.\n");
    parse_term(parser);
    parse_relation_term(parser);
    printf("Finished parsing relation.\n");
}

void parse_relation_term(parser_T* parser)
{
    switch(parser->look_ahead->type) {
        case T_LT:
            parser_eat(parser, T_LT);
            parse_term(parser);
            parse_relation_term(parser);
            break;
        case T_GTEQ:
            parser_eat(parser, T_GTEQ);
            parse_term(parser);
            parse_relation_term(parser);
            break;
        case T_LTEQ:
            parser_eat(parser, T_LTEQ);
            parse_term(parser);
            parse_relation_term(parser);
            break;
        case T_GT:
            parser_eat(parser, T_GT);
            parse_term(parser);
            parse_relation_term(parser);
            break;
        case T_EQ:
            parser_eat(parser, T_EQ);
            parse_term(parser);
            parse_relation_term(parser);
            break;
        case T_NOT_EQ:
            parser_eat(parser, T_NOT_EQ);
            parse_term(parser);
            parse_relation_term(parser);
            break;
        // FOLLOW set
        case T_PLUS: case T_MINUS: // arith op
        case T_AND: case T_OR: case T_COMMA: // expression
        case T_RPAREN: // for loop, if statement
        case T_RBRACKET: // assignment statement
        case T_SEMI_COLON: // statements
            break;
        default:
            printf("Error. Invalid relation parsing.\n");
            break;
        // default: break;
    }
}

void parse_term(parser_T* parser)
{
    printf("Parsing term.\n");
    parse_factor(parser);
    parse_term_factor(parser);
    printf("Finished parsing term.\n");
}

void parse_term_factor(parser_T* parser)
{
    switch(parser->look_ahead->type)
    {
        case T_MULTIPLY:
            parser_eat(parser, T_MULTIPLY);
            parse_factor(parser);
            parse_term_factor(parser);
            break;
        case T_DIVIDE:
            parser_eat(parser, T_DIVIDE);
            parse_factor(parser);
            parse_term_factor(parser);
            break;
        case T_LT: case T_LTEQ: case T_GT: case T_GTEQ: case T_EQ: case T_NOT_EQ: // relation
        case T_PLUS: case T_MINUS: // arith op
        case T_AND: case T_OR: case T_COMMA: // expression
        case T_RPAREN: // for loop, if statement
        case T_RBRACKET: // assignment statement
        case T_SEMI_COLON: // statements
            break;
        default:
            printf("Error. Invalid term parsing.\n");
            break;
    }
}

void parse_factor(parser_T* parser)
{
    printf("Parsing a factor.\n");
    switch(parser->look_ahead->type)
    {
        case T_LPAREN:  // <factor> ::= (<expression>)
            parser_eat(parser, T_LPAREN);
            parse_expression(parser);
            parser_eat(parser, T_RPAREN);
            break;
        case T_ID:  // <name> ::= <identifier> [ [ <expression> ] ]
            parser_eat(parser, T_ID);
            parse_indexes(parser);
            break;
        case T_NUMBER_INT:
            parser_eat(parser, T_NUMBER_INT);
            break;
        case T_STRING:
            parser_eat(parser, T_STRING);
            break;
        case T_MINUS: // [-] <name> | [-] <number>
            parser_eat(parser, T_MINUS);
            printf("A negative number of a negative name.\n");
            parse_factor(parser);
            break;
        case K_TRUE:
            parser_eat(parser, K_TRUE);
            break;
        case K_FALSE:
            parser_eat(parser, K_FALSE);
            break;
        case T_AND: case T_OR: case T_COMMA: // expression
        case T_RPAREN: // for loop, if statement
        case T_RBRACKET: // assignment statement
        case T_SEMI_COLON: // statements
            break;
        default:
            printf("Error. Invalid parsing factor.\n");
            break;
    }
}

// Function to parse array indexes
void parse_indexes(parser_T* parser)
{
    if (parser->look_ahead->type == T_LBRACKET)
    {
        parser_eat(parser, T_LBRACKET);
        parse_expression(parser);
        parser_eat(parser, T_RBRACKET);
        parse_indexes(parser); // 2D array
    }
}

void parse_param(parser_T* parser)
{
    printf("Parsing parameter.\n");
    parse_var_declaration(parser); // <parameter> ::= <variable declaration>
    printf("Finished parsing parameter.\n");
}

void parse_param_list(parser_T* parser)
{
    parse_param(parser);
    parse_param_list_param(parser);
}

void parse_param_list_param(parser_T* parser)
{
    switch(parser->look_ahead->type)
    {
        case T_COMMA:
            parser_eat(parser, T_COMMA);
            parse_param(parser);
            parse_param_list_param(parser);
            break;
        case T_RPAREN:
            break;
        default:
            printf("Error. Invalid parsing parameter list.\n");
            break;
    }
}