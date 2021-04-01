#include "include/parser.h"

/*
 * Parser constructor
 */
parser_T* init_parser(lexer_T* lexer)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->current_token = (void*) 0;
    parser->look_ahead = lexer_get_next_token(lexer);
    return parser;
}

/*
 * Eat/consume a token and look ahead the next one
 */
bool parser_eat(parser_T* parser, token_type type)
{
    Token* tmp = init_token(type);
    printf("Matching token. Expected type: ");
    printf("%s\n", print_token(tmp));

    if (!is_token_type(parser, type))
    {
        printf("Token doesn't match. Current look ahead is: %s\n", print_token(parser->look_ahead));
        return false;
    }
    else
    {
        printf("Token matched. Current look ahead is: %s\n", print_token(parser->look_ahead));
        parser->current_token = parser->look_ahead;
        parser->look_ahead = lexer_get_next_token(parser->lexer);
        return true;
    }
}

/*
 * Helper function checking for correct token type
 */
bool is_token_type(parser_T* parser, token_type type)
{
    if (parser->look_ahead->type != type)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool parse(parser_T* parser)
{
    return program(parser);
}

/*
 * <program> ::= <program_header> <program_body> .
 */
bool program(parser_T* parser)
{
    if (!program_header(parser))
    {
        return false;
    }

    if (!program_body(parser))
    {
        return false;
    }

    if (parser->look_ahead->type == T_EOF) parser_eat(parser, T_EOF);

    return true;
}

/*
 * <program_header> ::= program <identifier> is 
 */
bool program_header(parser_T* parser)
{
    parser_eat(parser, K_PROGRAM);

    if (!identifier(parser))
    {
        return false;
    }
    
    parser_eat(parser, K_IS);

    return true;
}

/* <program_body> ::=
 *          ( <declaration> ; )*
 *      begin
 *          ( <statement> ; )*
 *      end program
 */
bool program_body(parser_T* parser)
{
    if (!declaration_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_BEGIN))
    {
        return false;
    }

    if (!statement_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_END))
    {
        return false;
    }

    if (!parser_eat(parser, K_PROCEDURE))
    {
        return false;
    }

    return true;
}

/*
 * <declaration> ::=
 *      [ global ] <procedure_declaration>
 *    | [ global ] <variable_declaration>
 */
bool declaration(parser_T* parser)
{
    bool state;
    if (is_token_type(parser, K_GLOBAL))
    {
        parser_eat(parser, K_GLOBAL);
    }

    switch (parser->look_ahead->type)
    {
        case K_PROCEDURE:
            state = procedure_declaration(parser);
            break;
        case K_VARIABLE:
            state = variable_declaration(parser);
            break;
        default:
            state = false;
    }
    return state;
}

/*
 * <procedure_declaration> ::= <procedure_header> <procedure_body>
 */
bool procedure_declaration(parser_T* parser)
{
    if (!procedure_header(parser))
    {
        return false;
    }

    if (!procedure_body(parser))
    {
        return false;
    }

    return true;
}

/*
 * <procedure_header> ::=
 *      procedure <identifier> : <type_mark> ( [ <parameter_list> ] )
 */
bool procedure_header(parser_T* parser)
{
    if (!parser_eat(parser, K_PROCEDURE))
    {
        return false;
    }

    if (!identifier(parser))
    {
        return false;
    }

    if (!parser_eat(parser, T_COLON))
    {
        return false;
    }

    if (!type_mark(parser))
    {
        return false;
    }

    if (!parser_eat(parser, T_LPAREN))
    {
        return false;
    }

    // Optional parameter list
    parameter_list(parser);

    if (!parser_eat(parser, T_RPAREN))
    {
        return false;
    }
    return true;
}

/*
 * <parameter_list> ::=
 *      <parameter>, <parameter_list>
 *    | <parameter>
 */
bool parameter_list(parser_T* parser)
{
    if (!parameter(parser))
    {
        return false;
    }
    
    // Optional parameters
    while (is_token_type(parser, T_COMMA))
    {
        parser_eat(parser, T_COMMA);
        if (!parameter(parser))
        {
            return false;
        }
    }
    return true;
}

/*
 * <parameter> ::= <variable_declaration>
 */
bool parameter(parser_T* parser)
{
    return variable_declaration(parser);
}

/*
 * <procedure_body> ::=
 *          ( <declaration> ; )*
 *      begin
 *          ( <statement> ; )*
 *      end procedure
 */
bool procedure_body(parser_T* parser)
{
    if (!declaration_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_BEGIN))
    {
        return false;
    }

    if (!statement_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_END))
    {
        return false;
    }

    if (!parser_eat(parser, K_PROCEDURE))
    {
        return false;
    }
    return true;
}

/*
 * <variable_declaration> ::=
 *      variable <identifier> : <type_mark> [ [ <bound ] ]
 */
bool variable_declaration(parser_T* parser)
{
    if (!parser_eat(parser, K_VARIABLE))
    {
        return false;
    }

    if (!identifier(parser))
    {
        return false;
    }

    if (!type_mark(parser))
    {
        return false;
    }

    // Optional array type
    if (is_token_type(parser, T_LBRACKET))
    {
        parser_eat(parser, T_LBRACKET);
        if (!bound(parser))
        {
            return false;
        }

        if (!is_token_type(parser, T_RBRACKET))
        {
            return false;
        }
        parser_eat(parser, T_RBRACKET);
    }

    return true;
}

/*
 * <type_mark> ::=
 *      integer | float | string | bool
 */
bool type_mark(parser_T* parser)
{
    switch (parser->look_ahead->type)
    {
        case K_INT:
            parser_eat(parser, K_INT);
            break;
        case K_FLOAT:
            parser_eat(parser, K_FLOAT);
            break;
        case K_STRING:
            parser_eat(parser, K_STRING);
            break;
        case K_BOOL:
            parser_eat(parser, K_BOOL);
            break;
        default:
            return false;
    }
    return true;
}

/*
 */
bool bound(parser_T* parser)
{
    Token tmp = {
        .type = parser->look_ahead->type,
        .value = parser->look_ahead->value
    };

    if (tmp.value.intVal && tmp.type == T_NUMBER_INT)
    {
        return parser_eat(parser, T_NUMBER_INT);
    }
    else
    {
        return false;
    }
}

/*
 * <statement> ::=
 *      <assigment_statement>
 *    | <if_statement>
 *    | <loop_statement>
 *    | <return_statement>
 */
bool statement(parser_T* parser)
{
    bool state;
    if (assignment_statement(parser))
    {
        state = true;
    }
    else if (if_statement(parser))
    {
        state = true;
    }
    else if (loop_statement(parser))
    {
        state = true;
    }
    else if (return_statement(parser))
    {
        state = true;
    }
    else
    {
        state = false;
    }
    return state;
}

/*
 * <assignment_statement> ::= <destination> := <expression>
 */
bool assignment_statement(parser_T* parser)
{
    if (!destination(parser))
    {
        return false;
    }

    if (!parser_eat(parser, T_ASSIGNMENT))
    {
        return false;
    }

    if (!expression(parser))
    {
        return false;
    }

    return true;
}

/*
 * <destination> ::= <identifier> [ [ <expression> ] ]
 */
bool destination(parser_T* parser)
{
    if (!identifier(parser))
    {
        return false;
    }

    if (!array_index(parser))
    {
        return false;
    }
    return true;
}

/*
 * <if_statement> ::=
 *      if ( <expression> ) then ( <statement> ; )*
 *      [ else ( <statement> ; )* ]
 *      end if
 */
bool if_statement(parser_T* parser)
{
    if (!parser_eat(parser, K_IF))
    {
        return false;
    }

    if (parser_eat(parser, T_LPAREN))
    {
        return false;
    }

    if (!expression(parser))
    {
        return false;
    }

    if (!parser_eat(parser, T_RPAREN))
    {
        return false;
    }

    if (!parser_eat(parser, K_THEN))
    {
        return false;
    }

    if (!statement_list(parser))
    {
        return false;
    }

    // Optional else statement
    if (is_token_type(parser, K_ELSE))
    {
        parser_eat(parser, K_ELSE);
        if (!statement_list(parser))
        {
            return false;
        }
    }

    if (!parser_eat(parser, K_END))
    {
        return false;
    }

    if (!parser_eat(parser, K_IF))
    {
        return false;
    }
    return true;
}

/*
 * <loop_statement> ::=
 *      for ( <assignment_statement> ; <expression> )
 *          ( <statement> ; )*
 *      end for
 */
bool loop_statement(parser)
{
    if (!parser_eat(parser, K_FOR))
    {
        return false;
    }

    if (!parser_eat(parser, T_LPAREN))
    {
        return false;
    }

    if (!assignment_statement(parser))
    {
        return false;
    }

    if (!parser_eat(parser, T_SEMI_COLON))
    {
        return false;
    }

    if (!expression(parser))
    {
        return false;
    }

    if (!parser_eat(parser, T_RPAREN))
    {
        return false;
    }

    if (!statement_list(parser))
    {
        return false;
    }

    if (!parser_eat(parser, K_END))
    {
        return false;
    }

    if (!parser_eat(parser, K_FOR))
    {
        return false;
    }

    return true;
}

/*
 * <return_statement> ::= return <expression>
 */
bool return_statement(parser)
{
    if (!parser_eat(parser, K_RETURN))
    {
        return false;
    }

    if (!expression(parser))
    {
        return false;
    }

    return true;
}

/*
 * <identifier> ::= [a-zA-Z][a-zA-Z0-9_]*
 */ 
bool identifier(parser)
{
    return parser_eat(parser, T_ID);
}

/*
 * <expression> ::= [ not ] <arith_op> <expression_prime>
 */
bool expression(parser)
{
    // Optional NOT
    if (is_token_type(parser, K_NOT))
    {
        parser_eat(parser, K_NOT);
    }

    if (!arith_op(parser))
    {
        return false;
    }

    if (!expression_prime(parser))
    {
        return false;
    }

    return true;
}

/*
 * <expression_prime> ::=
 *      & <arith_op> <expression_prime>
 *    | | <arith_op> <expression_prime>
 *    | null
 */
bool expression_prime(parser_T* parser)
{
    switch (parser->look_ahead->type)
    {
        case T_AND:
            parser_eat(parser, T_AND);
            break;
        case T_OR:
            parser_eat(parser_eat, T_OR);
            break;
        // null case
        default:
            return true;
    }

    if (!arith_op(parser))
    {
        return false;
    }

    if (!expression_prime(parser))
    {
        return false;
    }

    return true;
}





/* 
 * A list of declaration for ( <declaration> ; )*
 */
bool declaration_list(parser_T* parser)
{
    // Zero or more declarations
    while (declaration(parser)) {
        if (!parser_eat(parser, T_SEMI_COLON)) {
            return false;
        }
    }
    return true;
}

/*
 * A list of statements for ( <statement> ; )*
 */
bool statement_list(parser_T* parser)
{
    // Zero or more statements
    while(statement(parser))
    {
        if (!parser_eat(parser, T_SEMI_COLON))
        {
            return false;
        }
    }
    return true;
}

