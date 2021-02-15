#include <stdio.h>
#include "include/lexer.h"


int main(int argc, char *argv[])
{
    Lexer* lexer = init_lexer(
        "begin\n"

        "x := getBool();\n"
        "if(x) then\n"
            "tmp := putString(\"T\");\n"
        "else"
            "tmp := putString(\"F\");\n"
        "end if;"

        "end program.\n"
    );

    Token* token = lexer_get_next_token(lexer);

    while (token->type != T_EOF)
    {
        printf("TOKEN(%d)\n", token->type);
        token = lexer_get_next_token(lexer);
    }
    return 0;
}