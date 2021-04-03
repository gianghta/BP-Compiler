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
    printf("%s", print_token(tmp));

    if (!is_token_type(parser, type))
    {
        printf("Token doesn't match. Current look ahead is: %s", print_token(parser->look_ahead));
        return false;
    }
    else
    {
        printf("Token matched. Current look ahead is: %s", print_token(parser->look_ahead));
        parser->current_token = parser->look_ahead;
        parser->look_ahead = lexer_get_next_token(parser->lexer);
        printf("Current token: %sLook ahead is: %s\n", print_token(parser->current_token), print_token(parser->look_ahead));
        return true;
    }

    free(tmp);
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

// Holy entry point
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

    if (!parser_eat(parser, K_PROGRAM))
    {
        return false;
    }

    if (!parser_eat(parser, T_EOF))
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
    printf("Start parsing declaration...\n");
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

    if (!parser_eat(parser, T_COLON))
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
    // bool state;
    switch (parser->look_ahead->type)
    {
        case K_IF:
           if_statement(parser);
           return true;
        case K_FOR:
            loop_statement(parser);
            return true;
        case K_RETURN:
            return_statement(parser);
            return true;
        default:
            return assignment_statement(parser);
    }
    return false;

    // if (assignment_statement(parser))
    // {
    //     state = true;
    // }
    // else if (if_statement(parser))
    // {
    //     state = true;
    // }
    // else if (loop_statement(parser))
    // {
    //     state = true;
    // }
    // else if (return_statement(parser))
    // {
    //     state = true;
    // }
    // else
    // {
    //     state = false;
    // }
    // return state;
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
    printf("Start parsing IF statement.\n");
    if (!parser_eat(parser, K_IF))
    {
        return false;
    }

    if (!parser_eat(parser, T_LPAREN))
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
bool loop_statement(parser_T* parser)
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
bool return_statement(parser_T* parser)
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
bool identifier(parser_T* parser)
{
    return parser_eat(parser, T_ID);
}

/*
 * <expression> ::= [ not ] <arith_op> <expression_prime>
 */
bool expression(parser_T* parser)
{   
    printf("Start parsing expression...\n");
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
            parser_eat(parser, T_OR);
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
 * <arith_op> ::= <relation> <arith_op_prime>
 */
bool arith_op(parser_T* parser)
{
    if (!relation(parser))
    {
        return false;
    }

    if (!arith_op_prime(parser))
    {
        return false;
    }
    return true;
}

/*
 * <arith_op_prime> ::=
 *      + <relation> <arith_op_prime>
 *    | - <relation> <arith_op_prime>
 *    | null
 */
bool arith_op_prime(parser_T* parser)
{
    switch (parser->look_ahead->type)
    {
        case T_PLUS:
            parser_eat(parser, T_PLUS);
            break;
        case T_MINUS:
            parser_eat(parser, T_MINUS);
            break;
        default:
            return true;
    }

    if (!relation(parser))
    {
        return false;
    }

    if (!arith_op_prime(parser))
    {
        return false;
    }

    return true;
}

/*
 * <relation> ::= <term> <relation_prime>
 */
bool relation(parser_T* parser)
{
    if (!term(parser))
    {
        return false;
    }

    if (!relation_prime(parser))
    {
        return false;
    }

    return true;
}

/*
 * <relation_prime> ::=
 *          <   <term> <relation_prime>
 *        | >=  <term> <relation_prime>
 *        | >   <term> <relation_prime>
 *        | <=  <term> <relation_prime>
 *        | ==  <term> <relation_prime>
 *        | !=  <term> <relation_prime>
 *        | null
 */
bool relation_prime(parser_T* parser)
{
    switch (parser->look_ahead->type)
    {
        case T_LT:
            parser_eat(parser, T_LT);
            break;
        case T_LTEQ:
            parser_eat(parser, T_LTEQ);
            break;
        case T_GT:
            parser_eat(parser, T_GT);
            break;
        case T_GTEQ:
            parser_eat(parser, T_GTEQ);
            break;
        case T_EQ:
            parser_eat(parser, T_EQ);
            break;
        case T_NOT_EQ:
            parser_eat(parser, T_NOT_EQ);
            break;
        default:
            return true;
    }

    if (!term(parser))
    {
        return false;
    }

    if (!relation_prime(parser))
    {
        return false;
    }

    return true;
}

/*
 * <term> ::= <factor> <term_prime>
 */
bool term(parser_T* parser)
{
    if (!factor(parser))
    {
        return false;
    }

    if (!term_prime(parser))
    {
        return false;
    }

    return true;
}

/*
 * <term_prime> ::=
 *       *  <factor> <term_prime>
 *     | /  <factor> <term_prime>
 *     | null
 */
bool term_prime(parser_T* parser)
{
    switch (parser->look_ahead->type)
    {
        case T_MULTIPLY:
            parser_eat(parser, T_MULTIPLY);
            break;
        case T_DIVIDE:
            parser_eat(parser, T_DIVIDE);
            break;
        default:
            return true;
    }

    if (!factor(parser))
    {
        return false;
    }

    if (!term_prime(parser))
    {
        return false;
    }

    return true;
}

/*
 * <factor> ::=
 *      ( <expression> )
 *    | <procedure_call>
 *    | [ - ] <name>
 *    | [ - ] <number>
 *    | <string>
 *    | true
 *    | false
 */
bool factor(parser_T* parser)
{
    if (parser_eat(parser, T_LPAREN))
    {
        if (!expression(parser))
        {
            return false;
        }

        if (!parser_eat(parser, T_RPAREN))
        {
            return false;
        }
        return true;
    }
    else if (procedure_call_or_name_handler(parser))
    {
        return true;
    }
    else if (parser_eat(parser, T_MINUS))
    {
        if (name(parser))
        {
            return true;
        }
        else if (number(parser))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (number(parser))
    {
        return true;
    }
    else if (string(parser))
    {
        return true;
    }
    else if (parser_eat(parser, K_TRUE))
    {
        return true;
    }
    else if (parser_eat(parser, K_FALSE))
    {
        return true;
    }
    else {
        return false;
    }
}

/*
 * Helper for procedure call and name
 * because they both start with an identifier
 * 
 * <procedure_call> ::= <identifier> ( [argument_list ] )
 * 
 * <name> ::= <identifier> [ [ <expression> ] ]
 */
bool procedure_call_or_name_handler(parser_T* parser)
{
    if (!identifier(parser))
    {
        return false;
    }

    switch (parser->look_ahead->type)
    {
        case T_LPAREN:
            parser_eat(parser, T_LPAREN);

            // Optional arguments
            argument_list(parser);

            if (!parser_eat(parser, T_RPAREN))
            {
                return false;
            }
        default:
            // Optional array index
            if (!array_index(parser))
            {
                return false;
            }
    }

    return true;
}

/*
 * <name> ::= <identifier> [ [ <expression> ] ]
 */
bool name(parser_T* parser)
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
 * Handler for array index [ [ <expression> ] ]
 */
bool array_index(parser_T* parser)
{
    if (parser_eat(parser, T_LBRACKET))
    {
        if (!expression(parser))
        {
            return false;
        }

        if (!parser_eat(parser, T_RBRACKET))
        {
            return false;
        }
    }
    return true;
}

/*
 * <argument_list> ::=
 *      <expression>, <argument_list>
 *    | <expression>
 */
bool argument_list(parser_T* parser)
{
    if (!expression(parser))
    {
        return false;
    }

    // Optional arguments
    while (is_token_type(parser, T_COMMA))
    {
        parser_eat(parser, T_COMMA);
        if (!expression(parser))
        {
            return false;
        }
    }
    return true;
}

/*
 * <number> ::= [0-9][0-9_]*[.[0-9_]*]
 */
bool number(parser_T* parser)
{
    if (is_token_type(parser, T_NUMBER_INT))
    {
        return parser_eat(parser, T_NUMBER_INT);
    }
    else if (is_token_type(parser, T_NUMBER_FLOAT))
    {
        return parser_eat(parser, T_NUMBER_FLOAT);
    }
    else {
        return false;
    }
}

/*
 * <string> :: = "[^"]*"
 */
bool string(parser_T* parser)
{
    return parser_eat(parser, T_STRING);
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

