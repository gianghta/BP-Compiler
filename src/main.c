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
        "end if;\n"
        "end program."
    );

    Token* token = lexer_get_next_token(lexer);

    while (token->type != T_EOF)
    {
        print_token(token);
        token = lexer_get_next_token(lexer);
    }
    return 0;
}