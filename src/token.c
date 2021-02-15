#include "include/token.h"
#include <stdlib.h>


Token* init_token(token_type type)
{
    Token* token = calloc(1, sizeof(Token));
    token->type = type;

    return token;
}