#include <include/parser.h>

parser_T* init_parser(lexer_T* lexer)
{
    parser_T* parser = calloc(1, sizeof(struct PARSER_STRUCT));
    parser->lexer = lexer;
    parser->current_token = (void*) 0;
    parser->look_ahead = lexer_get_next_token(lexer);
}

void match(Token* token, parser_T* parser)
{
    printf("Matching token. Expected type: ");
    printf("%s\n", print_token(token));

    if (parser->look_ahead->type != type)
    {
        printf("Not matched. Look ahead is: ");
    }
}