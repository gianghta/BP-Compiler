#include "error.h"

const char* file_name;

void set_file_name(const char* name)
{
    file_name = name;
}

void throw_error(char* msg, Token* tok)
{
    error_flag = true;
    printf("%s:\n", file_name);
    printf("ERROR: %s\n", msg);
}

void debug_parser_statement(char* msg, bool flag)
{
    if (flag)
    {
        printf("Debugging parser: %s\n", msg);
    }
}