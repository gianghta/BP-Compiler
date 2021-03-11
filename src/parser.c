#include <include/parser.h>

parser_T* init_parser(lexer_T* lexer)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->current_token = (void*) 0;
    parser->look_ahead = lexer_get_next_token(lexer);
}

void parser_eat(parser_T* parser, token_type type)
{
    Token* tmp = init_token(type);
    printf("Matching token. Expected type: ");
    printf("%s\n", print_token(tmp));

    if (parser->look_ahead != type)
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

    if (parser->look_ahead == T_EOF) parser_eat(parser, T_EOF);

    printf("Program finished. Parsing finished\n");
}

void parse_program_body(parser_T* parser)
{
    // program body
    if (parser->look_ahead != K_BEGIN)
    {
        parse_declarations(parser);
    }

    parser_eat(parser, K_BEGIN);

    if (parser->look_ahead != K_END)
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
    parser_eat(parser, K_INT);
    parser_eat(parser, T_LPAREN);
    if (parser->look_ahead->type != T_RPAREN)
    {
        parse_param_list(parser);
    }
    parser_eat(parser, T_LPAREN);

    if (parser->look_ahead->type != K_END)
    {
        parse_statements(parser);
    }

    parser_eat(parser, K_END);
    parser_eat(parser, K_PROCEDURE);
    printf("Finished parsing procedure\n");
}

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